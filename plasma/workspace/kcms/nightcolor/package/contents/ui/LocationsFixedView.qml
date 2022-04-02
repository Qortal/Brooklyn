/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.5 as QQC2
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.5 as KCM

Kirigami.FormLayout {
    twinFormLayouts: parentLayout


    NumberField {
        id: latitudeFixedField
        // Match combobox width
        Layout.minimumWidth: modeSwitcher.width
        Layout.maximumWidth: modeSwitcher.width
        Kirigami.FormData.label: i18n("Latitude:")
        validator: DoubleValidator {bottom: -90; top: 90; decimals: 10}
        backend: kcm.nightColorSettings.latitudeFixed
        onBackendChanged: {
            kcm.nightColorSettings.latitudeFixed = backend;
        }
        KCM.SettingStateBinding {
            configObject: kcm.nightColorSettings
            settingName: "LatitudeFixed"
            extraEnabledConditions: kcm.nightColorSettings.active
        }
    }

    NumberField {
        id: longitudeFixedField
        // Match combobox width
        Layout.minimumWidth: modeSwitcher.width
        Layout.maximumWidth: modeSwitcher.width
        Kirigami.FormData.label: i18n("Longitude:")
        validator: DoubleValidator {bottom: -180; top: 180; decimals: 10}
        backend: kcm.nightColorSettings.longitudeFixed
        onBackendChanged: {
            kcm.nightColorSettings.longitudeFixed = backend;
        }
        KCM.SettingStateBinding {
            configObject: kcm.nightColorSettings
            settingName: "LongitudeFixed"
            extraEnabledConditions: kcm.nightColorSettings.active
        }
    }

    QQC2.Button {
        text: i18n("Detect Location")
        // Match combobox width
        Layout.minimumWidth: modeSwitcher.width
        icon.name: "find-location"
        onClicked: {
            startLocator();
            latitudeFixedField.backend = locator.latitude;
            longitudeFixedField.backend = locator.longitude;
        }
    }

    // Inform about geolocation access on clicking detect
    QQC2.Label {
        enabled: activator.checked
        wrapMode: Text.Wrap
        Layout.maximumWidth: modeSwitcher.width
        text: i18n("The device's location will be detected using GPS (if available), or by sending network information to <a href=\"https://location.services.mozilla.com\">Mozilla Location Service</a>.")
        onLinkActivated: { Qt.openUrlExternally("https://location.services.mozilla.com"); }
        font: Kirigami.Theme.smallFont
    }
}
