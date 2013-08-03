// -*- qml -*-

/*!
 * This file is part of CameraPlus.
 *
 * Copyright (C) 2012-2013 Mohammed Sameer <msameer@foolab.org>
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

import QtQuick 2.0
import QtCamera 1.0

CameraToolIcon {
    id: button

    iconId: cameraTheme.videoSceneModeIcon(settings.videoSceneMode)

    property list<ToolsModelItem> toolsModel: [
        ToolsModelItem {icon: cameraTheme.videoSceneModeAutoIconId; value: Scene.Auto},
        ToolsModelItem {icon: cameraTheme.videoSceneModeNightIconId; value: Scene.Night}
    ]

    property CameraToolBarTools tools: CameraToolBarTools {
        CameraLabel {
            height: parent ? parent.height : 0
            text: qsTr("Scene")
            verticalAlignment: Text.AlignVCenter
        }

        Repeater {
            model: parent != null && parent.visible ? toolsModel : undefined

            delegate: CheckButton {
                iconId: icon
                onClicked: settings.videoSceneMode = value
                checked: settings.videoSceneMode == value
            }
        }
    }
}
