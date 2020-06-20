// -*- c++ -*-

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

#ifndef CAMERA_RESOURCES_H
#define CAMERA_RESOURCES_H

#include <QObject>

class CameraResources : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool acquired READ acquired NOTIFY acquiredChanged);
  Q_PROPERTY(bool hijacked READ hijacked NOTIFY hijackedChanged);
  Q_PROPERTY(bool scaleAcquired READ isScaleAcquired NOTIFY scaleAcquisitionChanged);

  Q_ENUMS(Mode);

public:
  typedef enum {
    None,
    Image,
    Video,
    Recording,
    Player,
  } Mode;

  CameraResources(QObject *parent = 0);
  ~CameraResources();

  Q_INVOKABLE bool acquire(const Mode& mode);

  bool acquired() const;
  bool hijacked() const;
  bool isScaleAcquired() const;

signals:
  void acquiredChanged();
  void hijackedChanged();
  void updated();
  void scaleAcquisitionChanged();
};

#endif /* CAMERA_RESOURCES_H */
