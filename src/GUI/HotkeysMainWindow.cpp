#include <QMenuBar>
#include <QHeaderView>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <QScrollArea>

#include "../Info.hpp"
#include "../Logger.hpp"
#include "../Parsers/CSFParser.hpp"

#include "ActionHotkeyWidget.hpp"
#include "GUIConfig.hpp"
#include "HotkeysMainWindow.hpp"

HotkeysMainWindow::HotkeysMainWindow(const QVariant& configuration, QWidget* parent)
    : QMainWindow(parent)
    , pFactionsButtonsGroup{new QButtonGroup{this}}
    , pEntitiesTreeWidget{new QTreeWidget}
    , pHotkeysArea{new QScrollArea}
    , pHotkeysPanelsWidget{nullptr}
    , pAboutDialog{nullptr}
{
    SetFactions();

    resize(1200, 800);
    ConfigureMenu();

    pEntitiesTreeWidget->header()->hide();
    // smooth scrolling
    pEntitiesTreeWidget->setVerticalScrollMode(QTreeWidget::ScrollMode::ScrollPerPixel);
    // icon size
    pEntitiesTreeWidget->setIconSize(QSize{GUIConfig::ICON_MIN_HEIGHT, GUIConfig::ICON_MIN_HEIGHT});
    // entitiesTreeWidget.setSpacing(GUIConfig::entityIconMinimumHeight * 0.1);

    connect(pEntitiesTreeWidget, &QTreeWidget::itemSelectionChanged, this, &HotkeysMainWindow::SetHotkeysPanelsWidget);

    QBoxLayout* ltFactions = nullptr;
    int factonsCount = factionVector.size();

    if (factonsCount == Faction::BASIC_FACTION_COUNT)
    {
        ltFactions = new QHBoxLayout();

        // only 3 sections with factions and subfactions, 4 in each
        for (int sectionIndex = 0; sectionIndex < Faction::BASIC_FACTION_COUNT; sectionIndex += 4)
        {
            QVBoxLayout* ltCurrentFaction    = new QVBoxLayout();
            QHBoxLayout* ltCurrentSubfaction = new QHBoxLayout();

            for (int i = 0; i < 4; ++i)
            {
                const Faction currFaction = factionVector.at(sectionIndex + i);

                QPushButton* factionButton = new QPushButton{currFaction.GetDisplayName()};

                connect(factionButton, &QPushButton::pressed, this, [=]()
                {
                    SetEntitiesList(currFaction.GetShortName());
                });

                pFactionsButtonsGroup->addButton(factionButton);
                
                if (i == 0) // main faction
                    ltCurrentFaction->addWidget(factionButton);
                else        // subfactions
                    ltCurrentSubfaction->addWidget(factionButton);
            }

            ltCurrentFaction->addLayout(ltCurrentSubfaction);
            ltFactions->addLayout(ltCurrentFaction);
        }
    }
    else
    {
        LOGMSG(QString("Unable to parse more than 12 factions. Found factions : ") + QString::number(factonsCount));
    }

    connect(pFactionsButtonsGroup, &QButtonGroup::idClicked, this, [=](int id)
    {
        // Take the focus from the buttons group
        pEntitiesTreeWidget->setFocus();

        // Select pressed faction button
        for (auto* button : pFactionsButtonsGroup->buttons())
        {
            if (button == pFactionsButtonsGroup->button(id))
                button->setDown(true);
            else
                button->setDown(false);
        }
    });

    // Fill all available space
    pHotkeysArea->setWidgetResizable(true);

    QVBoxLayout* ltBuildingConfiguration = new QVBoxLayout();
    ltBuildingConfiguration->addWidget(pHotkeysArea, 2);
    ltBuildingConfiguration->addWidget(new QScrollArea, 1);

    QHBoxLayout* ltContent = new QHBoxLayout();
    ltContent->addWidget(pEntitiesTreeWidget, 4);
    ltContent->addLayout(ltBuildingConfiguration, 7);

    QVBoxLayout* ltMain = new QVBoxLayout();
    ltMain->addLayout(ltFactions);
    ltMain->addLayout(ltContent);

    // main widget
    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(ltMain);
    setCentralWidget(centralWidget);

    // Set start faction
    const auto firstFactionButton = pFactionsButtonsGroup->button(-2);
    if (firstFactionButton != nullptr) firstFactionButton->click();
}

void HotkeysMainWindow::ConfigureMenu()
{
    QMenu* mnFileOptions = new QMenu(tr("File"));
    mnFileOptions->addAction(tr("Special"));
    menuBar()->addMenu(mnFileOptions);
    menuBar()->addAction(tr("View"));

    QMenu* mnSettingsOptions = new QMenu(tr("Settings"));
    menuBar()->addMenu(mnSettingsOptions);

    QAction* actAbout = new QAction(tr("About"));
    connect(actAbout, &QAction::triggered, this, &HotkeysMainWindow::OnAbout);

    mnSettingsOptions->addAction(actAbout);
}

void HotkeysMainWindow::SetEntitiesList(const QString& factionShortName)
{
    pEntitiesTreeWidget->clear();

    QMap<Config::GameObjectTypes, Faction::GameObject> goMap = GetFactionRef(factionShortName).GetTechTree();

    // Skip if there are no entities of that type
    if(goMap.isEmpty()) return;

    // Create sections for all faction entities types
    for(const auto& objectType : Config::ENTITIES_STRINGS)
    {
        // Create new section of tree list
        QTreeWidgetItem* newTopEntityItem = new QTreeWidgetItem();
        newTopEntityItem->setText(0, QCoreApplication::translate("QObject", objectType.toUtf8().constData()));

        // Decorate
        newTopEntityItem->setIcon(0, GUIConfig::GetEntityTypePixmap(Config::ENTITIES_STRINGS.key(objectType))
                                               .scaledToHeight(GUIConfig::ICON_SCALING_HEIGHT, Qt::SmoothTransformation));

        // Append entities to the section
        for (const auto& go : goMap)
        {
            QTreeWidgetItem* currentNewEntityItem = new QTreeWidgetItem();
            currentNewEntityItem->setText(0, CSFPARSER->GetStringValue(go.ingameName));
            currentNewEntityItem->setIcon(0, QPixmap::fromImage(GUIConfig::DecodeWebpIcon(go.iconName)));
            currentNewEntityItem->setData(0, Qt::UserRole, QVariant::fromValue(QPair{factionShortName, go.iconName}));
            newTopEntityItem->addChild(currentNewEntityItem);
        }

        // Add section to the list
        pEntitiesTreeWidget->addTopLevelItem(newTopEntityItem);
    }

    // Configure view
    pEntitiesTreeWidget->expandAll();
    pEntitiesTreeWidget->scrollToTop();

    // Skip if missing top item
    const auto firstTopLevelItem = pEntitiesTreeWidget->topLevelItem(0);
    if (firstTopLevelItem == nullptr) return;

    // Skip if missing first entity
    const auto firstEntity = firstTopLevelItem->child(0);
    if (firstEntity == nullptr) return;

    // Set start entity
    pEntitiesTreeWidget->setCurrentItem(firstEntity);
}

void HotkeysMainWindow::SetHotkeysPanelsWidget()
{
    // Skip if there are no selected items
    if (pEntitiesTreeWidget->selectedItems().isEmpty()) return;

    // Current single selected item
    QTreeWidgetItem* pItem = pEntitiesTreeWidget->selectedItems().first();

    // Skip if it's the top level section item
    for (int i = 0; i < pEntitiesTreeWidget->topLevelItemCount(); ++i)
        if (pItem == pEntitiesTreeWidget->topLevelItem(i)) return;

    const QPair<QString, QString> specialItemInfo = pItem->data(0, Qt::UserRole).value<QPair<QString, QString>>();

    const QString& factionShortName = specialItemInfo.first;
    const QString& gameObjectName   = specialItemInfo.second;

    // const QVector<QVector<QSharedPointer<EntityAction>>> entityPanels = factionsManager.GetEntityActionPanels(factionShortName, gameObjectName);
    const auto gameObjectKeyboardLayouts = GetFactionRef(factionShortName).GetKeyboardLayoutsByObjectName(gameObjectName);

    // Recreate panels widget
    if (pHotkeysPanelsWidget != nullptr) pHotkeysPanelsWidget->deleteLater();
    pHotkeysPanelsWidget = new QTabWidget();

    // Forget old hotkey widgets
    vHotkeyWidgets.clear();

    // Panel index
    int i = 0;

    for (const auto& currLayout : gameObjectKeyboardLayouts)
    {
        QSet<ActionHotkeyWidget*> currentPanelWidgets;
        QVBoxLayout* hotkeysLayout = new QVBoxLayout();

        for (const auto& currAction : currLayout)
        {
            ActionHotkeyWidget* actionHotkey = new ActionHotkeyWidget{CSFPARSER->GetClearName(currAction.hotkeyString), 
                                                                      QString::fromStdWString(std::wstring{CSFPARSER->GetHotkey(currAction.hotkeyString)}),
                                                                      currAction.iconName};

            // Remember widget
            currentPanelWidgets.insert(actionHotkey);

            connect(actionHotkey, &ActionHotkeyWidget::HotkeyChanged, this, [=](const QString& newHotkey)
            {
                // Set new hotkey
                SetActionHotkey(factionShortName, gameObjectName, currAction.iconName, newHotkey);

                // Highlight keys for entity
                HighlightKeys(factionShortName, gameObjectName);
            });

            hotkeysLayout->addWidget(actionHotkey);
        }

        // Remember hotkeys panel
        vHotkeyWidgets.append(currentPanelWidgets);

        // Highlight keys for entity
        HighlightKeys(factionShortName, gameObjectName);

        // Condense the actions at the top
        hotkeysLayout->addStretch(1);

        QWidget* panelScrollWidget = new QWidget();
        panelScrollWidget->setLayout(hotkeysLayout);
        pHotkeysPanelsWidget->addTab(panelScrollWidget, QString(tr("Layout %1")).arg(++i));
    }

    // If only one panel -> hide header
    if (pHotkeysPanelsWidget->count() <= 1) pHotkeysPanelsWidget->tabBar()->hide();

    pHotkeysPanelsWidget->setMinimumSize(pHotkeysPanelsWidget->sizeHint());
    pHotkeysArea->setWidget(pHotkeysPanelsWidget);
}

void HotkeysMainWindow::HighlightKeys(const QString& fctIconName, const QString& goIconName) const
{
    // Skip if no widgets
    if (vHotkeyWidgets.isEmpty()) return;

    // Change color if the current key is in collisions
    QVector<QSet<QString>> keysCollisions = {}; // TODO: Implement code to detect groups of the same hotkeys in 1 keyboard layout

    // If no collisions
    if (keysCollisions.isEmpty())
    {
        // Unhighlight all keys and quit
        for (const auto& panel : vHotkeyWidgets)
            for (auto& hotkeyWidget : panel)
                hotkeyWidget->HighlightKey(false);

        return;
    }

    // Panel index
    int i = -1;

    for (const auto& panel : vHotkeyWidgets)
    {
        // Increase panel index
        ++i;

        for (auto& hotkeyWidget : panel)
            if (keysCollisions.at(i).contains(hotkeyWidget->GetHotkey()))
                hotkeyWidget->HighlightKey(true);
            else
                hotkeyWidget->HighlightKey(false);
    }
}

void HotkeysMainWindow::SetFactions()
{
    for(const auto& elem : TECH_TREE_SOURCE.Query("$.TechTree").toArray())
        factionVector.push_back(Faction{elem.toObject()});
}

const Faction& HotkeysMainWindow::GetFactionRef(const QString& name)
{
    int tmp = 0;

    for(int i = 0; i < factionVector.count(); i++)
    {
        const Faction& elem = factionVector[i];
        
        if(elem.GetShortName() == name)
        {
            tmp = i;
            break;
        }
    }
    
    return factionVector.at(tmp);
}

void HotkeysMainWindow::SetActionHotkey(const QString& fctShortName, const QString& goName, const QString& actName, const QString& hk)
{
    for(Faction& fct : factionVector)
    {
        if(fct.GetShortName() == fctShortName)
        {
            fct.SetHotkey(goName, actName, hk);
            break;
        }
    }
}

void HotkeysMainWindow::OnAbout()
{
    // if dialog already exists
    if (pAboutDialog != nullptr)
    {
        pAboutDialog->activateWindow();
        return;
    }

    QVBoxLayout* authorsL = new QVBoxLayout();
    authorsL->addWidget(new QLabel{tr("Authors: ") + AUTHORS});

    QGridLayout* contentL = new QGridLayout();
    contentL->addLayout(authorsL, 0, 0);
    QLabel* pixmap = new QLabel();
    pixmap->setPixmap(QPixmap::fromImage(GUIConfig::DecodeDefaultWebpIcon()));
    contentL->addWidget(pixmap, 0, 1);
    QLabel* textL = new QLabel{tr("Program licensed by GNU GPL v3")};
    textL->setWordWrap(true);
    textL->setAlignment(Qt::AlignJustify);
    contentL->addWidget(textL, 1, 0);
    contentL->setSizeConstraint(QLayout::SetFixedSize);

    pAboutDialog = new QDialog{this};
    pAboutDialog->setWindowTitle(tr("About"));
    pAboutDialog->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pAboutDialog->setWindowFlags(pAboutDialog->windowFlags() &
                               ~Qt::WindowContextHelpButtonHint |
                                Qt::MSWindowsFixedSizeDialogHint);

    connect(pAboutDialog, &QDialog::finished, this, [this]()
    {
        pAboutDialog->deleteLater();
        pAboutDialog = nullptr;
    });

    QDialogButtonBox* buttons = new QDialogButtonBox{QDialogButtonBox::Ok,
                                                     Qt::Orientation::Horizontal,
                                                     pAboutDialog};

    connect(buttons, &QDialogButtonBox::accepted, pAboutDialog, &QDialog::accept);

    QHBoxLayout* buttonsL = new QHBoxLayout();
    buttonsL->addStretch();
    buttons->button(QDialogButtonBox::Ok)->setFixedWidth(100);
    buttonsL->addWidget(buttons->button(QDialogButtonBox::Ok));
    buttonsL->addStretch();
    buttonsL->setAlignment(Qt::AlignCenter);

    QVBoxLayout* mainL = new QVBoxLayout();
    mainL->addLayout(contentL);
    mainL->addLayout(buttonsL);

    pAboutDialog->setLayout(mainL);
    pAboutDialog->show();
    pAboutDialog->raise();
    pAboutDialog->activateWindow();
}
