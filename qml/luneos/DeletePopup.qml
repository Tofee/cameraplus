// -*- qml -*-

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

import QtQuick 2.12

MouseArea {
    id: dialog

    property alias file: message.text
    property Item item
    property Item page

    z: 1
    anchors.fill: parent
    parent: visible ? page : item
    visible: opacity > 0
    enabled: visible
    opacity: 0

    Behavior on opacity {
        NumberAnimation { duration: 100 }
    }

    function open() {
        opacity = 1
    }

    function close() {
        opacity = 0
    }

    Rectangle {
        anchors.fill: parent
        color: cameraStyle.backgroundColor
        opacity: 0.8
    }

    Column {
        anchors.centerIn: parent
        spacing: cameraStyle.spacingLarge

        CameraLabel {
            width: parent.width
            font.pixelSize: cameraStyle.fontSizeLarge
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Delete item?")
        }

        CameraLabel {
            id: message
            width: parent.width
            font.pixelSize: cameraStyle.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
        }

        CameraButton {
            text: qsTr("Yes")
            onClicked: {
                item.deleteUrl()
                dialog.close()
            }
        }

        CameraButton {
            text: qsTr("No")
            onClicked: {
                dialog.close()
            }
        }
    }
}
