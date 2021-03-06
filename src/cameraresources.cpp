/*!
 * This file is part of CameraPlus.
 *
 * Copyright (C) 2012-2014 Mohammed Sameer <msameer@foolab.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "cameraresources.h"
#if defined(QT4)
#include <dbusconnectioneventloop.h>
#endif
#include <QDebug>

#define APPLICATION_CLASS "camera"

CameraResources::CameraResources(QObject *parent) :
  QObject(parent),
  m_worker(new CameraResourcesWorker) {

  m_worker->moveToThread(&m_thread);

#if defined(QT4)
  DBUSConnectionEventLoop::getInstance().moveToThread(&m_thread);
#endif

  QObject::connect(&m_thread, SIGNAL(started()), m_worker, SLOT(init()));
  m_thread.start();

  qRegisterMetaType<CameraResources::Mode>("CameraResources::Mode");
  qRegisterMetaType<bool *>("bool *");

  QObject::connect(m_worker, SIGNAL(acquiredChanged()), this, SIGNAL(acquiredChanged()));
  QObject::connect(m_worker, SIGNAL(hijackedChanged()), this, SIGNAL(hijackedChanged()));
  QObject::connect(m_worker, SIGNAL(updated()), this, SIGNAL(updated()));
  QObject::connect(m_worker, SIGNAL(acquiredChanged()), this, SIGNAL(scaleAcquisitionChanged()));
  QObject::connect(m_worker, SIGNAL(hijackedChanged()), this, SIGNAL(scaleAcquisitionChanged()));
  QObject::connect(m_worker, SIGNAL(updated()), this, SIGNAL(scaleAcquisitionChanged()));
}

CameraResources::~CameraResources() {
  acquire(CameraResources::None);
  m_thread.exit(0);

  while (m_thread.isRunning()) {
    m_thread.wait(10);
  }

  delete m_worker;
  m_worker = 0;
}

bool CameraResources::isResourceGranted(const ResourcePolicy::ResourceType& resource) const {
  bool ok = false;

  QMetaObject::invokeMethod(m_worker, "isResourceGranted", Qt::BlockingQueuedConnection,
			    Q_ARG(bool *, &ok), Q_ARG(int, resource));

  return ok;
}

bool CameraResources::acquire(const Mode& mode) {
  bool ok = false;

  QMetaObject::invokeMethod(m_worker, "acquire", Qt::BlockingQueuedConnection,
			    Q_ARG(bool *, &ok), Q_ARG(CameraResources::Mode, mode));

  return ok;
}

bool CameraResources::acquired() const {
  bool ok = false;

  QMetaObject::invokeMethod(m_worker, "acquired", Qt::BlockingQueuedConnection,
			    Q_ARG(bool *, &ok));

  return ok;
}

bool CameraResources::hijacked() const {
  bool ok = false;

  QMetaObject::invokeMethod(m_worker, "hijacked", Qt::BlockingQueuedConnection,
			    Q_ARG(bool *, &ok));

  return ok;
}

bool CameraResources::isScaleAcquired() const {
  return isResourceGranted(ResourcePolicy::ScaleButtonType);
}

CameraResourcesWorker::CameraResourcesWorker(QObject *parent) :
  QObject(parent),
  m_set(0),
  m_mode(CameraResources::None),
  m_acquired(false),
  m_acquiring(false),
  m_hijacked(false) {

}

CameraResourcesWorker::~CameraResourcesWorker() {

}

void CameraResourcesWorker::init() {
  m_set = new ResourcePolicy::ResourceSet(APPLICATION_CLASS, this);
  m_set->setAlwaysReply();

  QObject::connect(m_set, SIGNAL(resourcesReleased()), this, SLOT(resourcesReleased()));
  QObject::connect(m_set, SIGNAL(lostResources()), this, SLOT(lostResources()));
  QObject::connect(m_set, SIGNAL(resourcesGranted(const QList<ResourcePolicy::ResourceType>&)),
		   this, SLOT(resourcesGranted(const QList<ResourcePolicy::ResourceType>&)));
  QObject::connect(m_set, SIGNAL(resourcesDenied()), this, SLOT(resourcesDenied()));

  if (!m_set->initAndConnect()) {
    qCritical() << "Failed to connect to resource policy engine";
  }
}

void CameraResourcesWorker::resourcesReleased() {
  setHijacked(false);
  setAcquired(false);

  m_acquiring = false;
}

void CameraResourcesWorker::lostResources() {
  setAcquired(false);
  setHijacked(true);

  m_acquiring = false;
}

void CameraResourcesWorker::resourcesGranted(const QList<ResourcePolicy::ResourceType>& optional) {
  Q_UNUSED(optional);

  if (!m_acquiring) {
    // This can happen when:
    // 1) We lose/gain optional resources.
    // 2) A higher priority application releases resources back.
    emit updated();
  }

  m_acquiring = false;

  setAcquired(true);
  setHijacked(false);
}

void CameraResourcesWorker::resourcesDenied() {
  setAcquired(false);
  setHijacked(true);

  m_acquiring = false;
}

QList<ResourcePolicy::ResourceType> CameraResourcesWorker::listSet() {
  QList<ResourcePolicy::Resource *> resources = m_set->resources();

  QList<ResourcePolicy::ResourceType> set;

  foreach (ResourcePolicy::Resource *r, resources) {
    set << r->type();
  }

  return set;
}

void CameraResourcesWorker::acquire(bool *ok, const CameraResources::Mode& mode) {
  if (mode == m_mode) {
    *ok = true;
    return;
  }

  m_mode = mode;

  *ok = false;

  switch (m_mode) {
  case CameraResources::None:
    *ok = release();
    break;

  case CameraResources::Image:
    *ok = updateSet(QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::VideoPlaybackType
		    << ResourcePolicy::VideoRecorderType,
		    QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::ScaleButtonType);
    break;

  case CameraResources::Video:
    *ok = updateSet(QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::VideoPlaybackType
		    << ResourcePolicy::VideoRecorderType,
		    QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::ScaleButtonType);
    break;

  case CameraResources::Recording:
    *ok = updateSet(QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::VideoPlaybackType
		    << ResourcePolicy::VideoRecorderType,
		    QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::ScaleButtonType
		    << ResourcePolicy::AudioRecorderType
		    << ResourcePolicy::AudioPlaybackType);
    break;

  case CameraResources::Player:
    *ok = updateSet(QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::VideoPlaybackType
		    << ResourcePolicy::AudioPlaybackType,
		    QList<ResourcePolicy::ResourceType>()
		    << ResourcePolicy::ScaleButtonType);
    break;

  default:
    qWarning() << "Unknown mode" << mode;

    *ok = false;
  }
}

void CameraResourcesWorker::acquired(bool *ok) {
  *ok = m_acquired;
}

void CameraResourcesWorker::hijacked(bool *ok) {
  *ok = m_hijacked;
}

bool CameraResourcesWorker::updateSet(const QList<ResourcePolicy::ResourceType>& required,
				      const QList<ResourcePolicy::ResourceType>& optional) {


  QList<ResourcePolicy::ResourceType> set = listSet();

  foreach (ResourcePolicy::ResourceType r, set) {
    if (required.indexOf(r) != -1) {
      m_set->resource(r)->setOptional(false);
    }
    else if (optional.indexOf(r) != -1) {
      m_set->resource(r)->setOptional(true);
    }
    else {
      m_set->deleteResource(r);
    }
  }

  foreach (ResourcePolicy::ResourceType r, required) {
    m_set->addResource(r);
  }

  foreach (ResourcePolicy::ResourceType r, optional) {
    m_set->addResource(r);
    ResourcePolicy::Resource *res = m_set->resource(r);
    if (res) {
      res->setOptional(true);
    }
  }

  if (m_set->contains(ResourcePolicy::AudioPlaybackType)) {
    bool isOptional = m_set->resource(ResourcePolicy::AudioPlaybackType)->isOptional();

    ResourcePolicy::AudioResource *audio = new ResourcePolicy::AudioResource(APPLICATION_CLASS);
    audio->setProcessID(QCoreApplication::applicationPid());
    audio->setStreamTag("media.name", "*");
    audio->setOptional(isOptional);
    m_set->addResourceObject(audio);
  }

  m_acquiring = true;

  m_set->update();
  m_set->acquire();

  while (m_acquiring) {
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
  }

  return m_acquired;
}

bool CameraResourcesWorker::release() {
  m_acquiring = true;

  m_set->release();

  while (m_acquiring) {
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
  }

  m_mode = CameraResources::None;

  return true;
}

void CameraResourcesWorker::setAcquired(bool acquired) {
  if (m_acquired != acquired) {
    m_acquired = acquired;
    emit acquiredChanged();
  }
}

void CameraResourcesWorker::setHijacked(bool hijacked) {
  if (m_hijacked != hijacked) {
    m_hijacked = hijacked;
    emit hijackedChanged();
  }
}

void CameraResourcesWorker::isResourceGranted(bool *ok, int resource) {

  ResourcePolicy::ResourceType rt = (ResourcePolicy::ResourceType)resource;

  ResourcePolicy::Resource *r = m_set->resource(rt);
  if (r) {
    *ok = r->isGranted();
  }
}
