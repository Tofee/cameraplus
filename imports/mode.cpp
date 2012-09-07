#include "mode.h"
#include "qtcammode.h"
#include "camera.h"
#include "qtcamdevice.h"
#include "previewprovider.h"

Mode::Mode(QObject *parent) :
  QObject(parent),
  m_cam(0),
  m_mode(0),
  m_seq(0) {

}

Mode::~Mode() {
  m_cam = 0;
  m_mode = 0;
}

Camera *Mode::camera() {
  return m_cam;
}

void Mode::setCamera(Camera *camera) {
  if (camera == m_cam) {
    return;
  }

  if (m_cam) {
    QObject::disconnect(m_cam, SIGNAL(deviceChanged()), this, SLOT(deviceChanged()));
  }

  m_cam = camera;

  if (m_cam) {
    QObject::connect(m_cam, SIGNAL(deviceChanged()), this, SLOT(deviceChanged()));
  }

  emit cameraChanged();

  deviceChanged();
}

bool Mode::isActive() {
  return m_mode ? m_mode->isActive() : false;
}

bool Mode::canCapture() {
  return m_mode ? m_mode->canCapture() : false;
}

void Mode::deviceChanged() {
  if (m_mode) {
    QObject::disconnect(m_mode, SIGNAL(canCaptureChanged()), this, SIGNAL(canCaptureChanged()));
    QObject::disconnect(m_mode, SIGNAL(saved(const QString&)),
			this, SIGNAL(saved(const QString&)));
    QObject::disconnect(m_mode, SIGNAL(previewAvailable(const QImage&, const QString&)),
			this, SLOT(gotPreview(const QImage&, const QString&)));
    QObject::disconnect(m_mode, SIGNAL(activeChanged()), this, SIGNAL(activeChanged()));
    QObject::disconnect(m_mode, SIGNAL(activeChanged()), this, SIGNAL(canCaptureChanged()));
    QObject::disconnect(m_cam->device(), SIGNAL(idleStateChanged(bool)),
			this, SIGNAL(canCaptureChanged()));
    QObject::disconnect(m_cam->device(), SIGNAL(runningStateChanged(bool)),
			this, SIGNAL(canCaptureChanged()));
    QObject::disconnect(m_mode, SIGNAL(nightModeChanged()), this, SIGNAL(nightModeChanged()));

    preChangeMode();
  }

  if (!m_cam || !m_cam->device()) {
    return;
  }

  changeMode();

  if (m_mode) {
    QObject::connect(m_mode, SIGNAL(canCaptureChanged()), this, SIGNAL(canCaptureChanged()));
    QObject::connect(m_mode, SIGNAL(saved(const QString&)), this, SIGNAL(saved(const QString&)));
    QObject::connect(m_mode, SIGNAL(previewAvailable(const QImage&, const QString&)),
		     this, SLOT(gotPreview(const QImage&, const QString&)));
    QObject::connect(m_mode, SIGNAL(activeChanged()), this, SIGNAL(activeChanged()));
    QObject::connect(m_mode, SIGNAL(activeChanged()), this, SIGNAL(canCaptureChanged()));
    QObject::connect(m_cam->device(), SIGNAL(idleStateChanged(bool)),
    		     this, SIGNAL(canCaptureChanged()));
    QObject::connect(m_cam->device(), SIGNAL(runningStateChanged(bool)),
		     this, SIGNAL(canCaptureChanged()));
    QObject::connect(m_mode, SIGNAL(nightModeChanged()), this, SIGNAL(nightModeChanged()));

    postChangeMode();
  }

  emit canCaptureChanged();
  emit activeChanged();
}

void Mode::gotPreview(const QImage& image, const QString& fileName) {
  PreviewProvider::instance()->setPreview(image);

  // HACK: QML insists on caching the images.
  QString url = QString("image://preview/%1").arg(m_seq);
  ++m_seq;

  emit previewAvailable(url, fileName);
}

void Mode::setNightMode(bool night) {
  if (m_mode) {
    m_mode->setNightMode(night);
  }
}

bool Mode::inNightMode() const {
  return m_mode ? m_mode->inNightMode() : false;
}