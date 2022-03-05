/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "kcm/configcontainer.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(ConfigContainer, "mouse.json")

#include <plugin.moc>
