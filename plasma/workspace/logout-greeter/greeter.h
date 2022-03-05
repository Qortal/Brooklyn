/*
    ksmserver - the KDE session management server

    SPDX-FileCopyrightText: 2016 Martin Graesslin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: MIT
*/

#pragma once

#include <QObject>
#include <QVector>

#include <kworkspace.h>

class KSMShutdownDlg;

class QScreen;

class Greeter : public QObject
{
    Q_OBJECT
public:
    Greeter();
    ~Greeter() override;

    void init();

    bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:
    void promptLogout();
    void promptShutDown();
    void promptReboot();

private:
    void adoptScreen(QScreen *screen);
    void rejected();
    void setupWaylandIntegration();

    bool m_running = false;

    KWorkSpace::ShutdownType m_shutdownType = KWorkSpace::ShutdownTypeHalt;
    QVector<KSMShutdownDlg *> m_dialogs;
};
