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

#include "panorama.h"
#if defined(QT4)
#include <QDeclarativeInfo>
#elif defined(QT5)
#include <QQmlInfo>
#endif
#include "panoramainput.h"
#include "panoramatracker.h"
#include "panoramastitcher.h"

Panorama::Panorama(QObject *parent) :
  QObject(parent),
  m_input(0),
  m_tracker(0),
  m_stitcher(0),
  m_keepFrames(false) {

}

Panorama::~Panorama() {
  m_lock.lock();
  if (m_tracker) {
    m_tracker->stop();
    m_tracker->deleteLater();
    m_tracker = 0;
  }

  if (m_stitcher) {
    m_stitcher->stop();
    m_stitcher->deleteLater();
    m_stitcher = 0;
  }
}

int Panorama::frameCount() {
  QMutexLocker l(&m_lock);

  return m_tracker ? m_tracker->frameCount() : 0;
}

int Panorama::stitchingProgress() {
  QMutexLocker l(&m_lock);
  return m_stitcher ? m_stitcher->progress() : 0;
}

Panorama::Status Panorama::status() {
  QMutexLocker l(&m_lock);
  return m_stitcher ? Panorama::Stitching : m_tracker ? Panorama::Tracking : Panorama::Idle;
}

PanoramaInput *Panorama::input() const {
  return m_input;
}

void Panorama::setInput(PanoramaInput *input) {
  if (m_input != input) {
    m_input = input;
    emit inputChanged();
  }
}

void Panorama::start(const QString& output) {
  if (status() != Panorama::Idle) {
    qmlInfo(this) << "Already running";
    return;
  }

  m_lock.lock();
  m_input->start();

  m_output = output;
  m_tracker = new PanoramaTracker(m_input);
  QObject::connect(m_tracker, SIGNAL(error(const Panorama::Error&)),
		   this, SIGNAL(error(const Panorama::Error&)));
  QObject::connect(m_tracker, SIGNAL(frameCountChanged()), this, SLOT(trackerFrameCountChanged()));
  m_tracker->start();
  m_lock.unlock();

  emit statusChanged();
}

void Panorama::stop() {
  m_lock.lock();

  // This first to unlock tracker
  m_input->stop();

  if (m_tracker) {
    m_tracker->stop();
    m_tracker->deleteLater();
    m_tracker = 0;
  }

  if (m_stitcher) {
    m_stitcher->stop();
    m_stitcher->deleteLater();
    m_stitcher = 0;
  }

  m_lock.unlock();

  emit statusChanged();
}

void Panorama::stitch() {
  m_lock.lock();

  if (!m_tracker) {
    qmlInfo(this) << "we are not tracking";
    m_lock.unlock();
    return;
  }

  if (m_stitcher) {
    qmlInfo(this) << "We are already stitching";
    m_lock.unlock();
    return;
  }

  // This first to unlock tracker
  m_input->stop();

  m_tracker->stop();

  std::vector<guint8 *> frames;
  m_tracker->releaseFrames(frames);

  m_stitcher = new PanoramaStitcher(frames, m_tracker->size(), m_output, m_keepFrames);
  QObject::connect(m_stitcher, SIGNAL(progressChanged()),
		   this, SIGNAL(stitchingProgressChanged()));
  QObject::connect(m_stitcher, SIGNAL(error(const Panorama::Error&)),
		   this, SIGNAL(error(const Panorama::Error&)));
  QObject::connect(m_stitcher, SIGNAL(done()), this, SLOT(stitchingDone()));
  m_stitcher->start();

  m_tracker->deleteLater();
  m_tracker = 0;

  m_lock.unlock();

  emit statusChanged();
}

void Panorama::clear(std::vector<guint8 *>& v) {
  for (int x = 0; x < v.size(); x++) {
    delete [] v[x];
  }

  v.clear();
}

void Panorama::trackerFrameCountChanged() {
  emit frameCountChanged();

  if (frameCount() >= MAX_TRACKER_FRAMES) {
    stitch();
  }
}

void Panorama::stitchingDone() {
  m_lock.lock();
  if (m_stitcher) {
    m_stitcher->stop();
    m_stitcher->deleteLater();
    m_stitcher = 0;
  }

  m_lock.unlock();

  emit statusChanged();

  emit stitchingProgressChanged();
}

bool Panorama::keepFrames() const {
  return m_keepFrames;
}

void Panorama::setKeepFrames(bool keep) {
  if (m_keepFrames != keep) {
    m_keepFrames = keep;
    emit keepFramesChanged();
  }
}
