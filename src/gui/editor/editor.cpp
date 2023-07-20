#include <QDebug>
#include <QMenuBar>
#include <QApplication>
#include <QTranslator>
#include <QScrollArea>

#include "editor.hpp"
#include "hotkeyelement.hpp"

Editor::Editor(Helper::Games game, bool saveToGame, QWidget *parent)
    : QMainWindow(parent)
{
    // configuring menu
    QMenu* fm = new QMenu(tr("File"));
    fm->addAction(tr("Special"));
    menuBar()->addMenu(fm);
    menuBar()->addAction(tr("View"));
    QMenu* settingsM = new QMenu(tr("Settings"));
    menuBar()->addMenu(settingsM);

//    QWidget* hotkeysWidget = new QWidget;
    QVBoxLayout* mainL = new QVBoxLayout;
//    for(int i = 0; i < 7; i++)
//        mainL->addWidget(new HotkeyElement(QString("action_%1").arg(i+1),
//                                           QString("hotkey_%1").arg(i+1),
//                                           QString("sources/icons/Gen1_Hacker_Icons.webp")));
//    hotkeysWidget->setLayout(mainL);

//    QScrollArea* scrollArea = new QScrollArea;
//    scrollArea->setWidget(hotkeysWidget);

//    setCentralWidget(scrollArea);

    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(mainL);
    setCentralWidget(centralWidget);
}
