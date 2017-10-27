/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>
#include <QFontDatabase>
#include <QTextCodec>
#include <QDebug>

#include <option.h>
#include <settings.h>

#include "dsettingsdialog.h"

#include "dslider.h"
#include "dthememanager.h"
#include "dwidget_global.h"
#include "dswitchbutton.h"
#include "segmentedcontrol.h"
#include "dcolorpicker.h"
#include "dplatformwindowhandle.h"
#include "dtitlebar.h"

#include <DApplication>

#include "mainwindow.h"
#include "buttonlisttab.h"
#include "buttongridtab.h"
#include "imagebuttongridtab.h"
#include "cameraform.h"
#include "graphicseffecttab.h"

DTK_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    DThemeManager *themeManager = DThemeManager::instance();

    initTabWidget();

    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->setMargin(5);
    mainLayout->addWidget(m_mainTab);

    QHBoxLayout *styleLayout = new QHBoxLayout();
    QPushButton *darkButton = new QPushButton("Dark", this);
    QPushButton *lightBUtton = new QPushButton("Light", this);

    themeManager->setTheme(lightBUtton, "light");

    connect(darkButton, &QPushButton::clicked, [ = ] {
        themeManager->setTheme("dark");
    });
    connect(lightBUtton, &QPushButton::clicked, [ = ] {
        themeManager->setTheme("light");
    });
    styleLayout->addWidget(darkButton);
    styleLayout->addWidget(lightBUtton);
    styleLayout->addStretch();

    mainLayout->addLayout(styleLayout);

    QWidget *centralWidget = new QWidget(this);

    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);

    DTitlebar *titlebar = this->titleBar();

    if (titlebar) {
        titlebar->setMenu(new QMenu(titlebar));
        titlebar->setSeparatorVisible(true);
        titlebar->menu()->addAction("dfm-settings");
        titlebar->menu()->addAction("dt-settings");
        titlebar->menu()->addAction("testmenu1");
        titlebar->menu()->addAction("testmenu2");
        QMenu *menu = titlebar->menu()->addMenu("menu1");
        menu->addAction("menu1->action1");
        menu->addAction("menu1->action2");

        connect(titlebar->menu(), &QMenu::triggered, this, &MainWindow::menuItemInvoked);
    }
}

#include <QTemporaryFile>
#include <qsettingbackend.h>

void MainWindow::menuItemInvoked(QAction *action)
{
    if (action->text() == "dfm-settings") {
        QTemporaryFile tmpFile;
        tmpFile.open();
        auto backend = new QSettingBackend(tmpFile.fileName());

        auto settings = Settings::fromJsonFile(":/resources/data/dfm-settings.json");
        settings->setBackend(backend);

        DSettingsDialog dsd(this);
        dsd.updateSettings(settings);
        dsd.exec();
        return;
    }

    if (action->text() == "dt-settings") {
        QTemporaryFile tmpFile;
        tmpFile.open();
        auto backend = new QSettingBackend(tmpFile.fileName());

        auto settings = Settings::fromJsonFile(":/resources/data/dt-settings.json");
        settings->setBackend(backend);

        QFontDatabase fontDatabase;
        auto fontFamliy = settings->option("base.font.family");
        fontFamliy->setData("items", fontDatabase.families());
        fontFamliy->setValue(0);

        QStringList codings;
        for (auto coding: QTextCodec::availableCodecs())
            codings << coding;

        auto encoding = settings->option("advance.encoding.encoding");
        encoding->setData("items", codings);
        encoding->setValue(0);

        DSettingsDialog dsd(this);
        dsd.updateSettings(settings);
        dsd.exec();
        return;
    }

    QMessageBox::warning(this, "menu clieck",  action->text() + ", was cliecked");
    qDebug() << "click" << action << action->isChecked();
}

void MainWindow::initTabWidget()
{
    m_mainTab = new QTabWidget(this);

    ComboBoxTab *comboBoxTab = new ComboBoxTab(this);

    LineTab *lineTab = new LineTab(this);

    BarTab *barTab = new BarTab(this);

    ButtonTab *buttonTab = new ButtonTab(this);

    InputTab *inputTab = new InputTab(this);

    SliderTab *sliderTab = new SliderTab(this);

    IndicatorTab *indicatorTab = new IndicatorTab(this);

    ButtonListTab *buttonListGroupTab = new ButtonListTab(this);

    ButtonGridTab *buttonGridTab = new ButtonGridTab(this);

    ImageButtonGridTab *imageButtonGridTab = new ImageButtonGridTab(this);

    Segmentedcontrol *segmentedControl = new Segmentedcontrol(this);

    WidgetsTab *widgetsTab = new WidgetsTab(this);

    DColorPicker *picker = new DColorPicker(1, this);

    CameraForm *cameraform = new CameraForm(this);

    GraphicsEffectTab *effectTab = new GraphicsEffectTab(this);

    m_mainTab->addTab(widgetsTab, "Widgets");
    m_mainTab->addTab(effectTab, "GraphicsEffect");
    m_mainTab->addTab(comboBoxTab, "ComboBox");
    m_mainTab->addTab(indicatorTab, "Indicator");
    m_mainTab->addTab(lineTab, "Line");
    m_mainTab->addTab(barTab, "Bar");
    m_mainTab->addTab(buttonTab, "Button");
    m_mainTab->addTab(inputTab, "Input");
    m_mainTab->addTab(sliderTab, "Slider");

    m_mainTab->addTab(buttonListGroupTab, "ButtonList");
    m_mainTab->addTab(imageButtonGridTab, "imageButtonGrid");
    m_mainTab->addTab(buttonGridTab, "ButtonGrid");
    m_mainTab->addTab(segmentedControl, "Segmented Control");
    m_mainTab->addTab(picker, "Color Picker");
    m_mainTab->addTab(cameraform, "Camera View");

    m_mainTab->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{

}
