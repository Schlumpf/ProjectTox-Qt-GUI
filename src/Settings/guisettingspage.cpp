#include "guisettingspage.h"
#include "settings.hpp"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

GuiSettingsPage::GuiSettingsPage(QWidget *parent) :
    AbstractSettingsPage(parent)
{
}

void GuiSettingsPage::buildGui()
{
    // Language
    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *languageGroup  = buildLanguageGroup();
    QGroupBox *animationGroup = buildAnimationGroup();

    layout->addWidget(animationGroup);
    layout->addWidget(languageGroup);
    layout->addStretch(0);
}

void GuiSettingsPage::setGui()
{
    const Settings& settings = Settings::getInstance();
    const QVariant language = settings.getGuiLanguage();
    langCombo->setCurrentIndex(langCombo->findData(language));

    connect(langCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(showRestartInfo(int)));
    //connect(langCombo, &QComboBox::currentIndexChanged, this, &GuiSettingsPage::showRestartInfo);

    enableAnimationCheckbox->setChecked(settings.isAnimationEnabled());
}

void GuiSettingsPage::showRestartInfo(int i)
{
    Q_UNUSED(i)
    Settings& settings = Settings::getInstance();
    if(langCombo->itemData(langCombo->currentIndex()) == settings.getGuiLanguage())
        restartLabel->hide();
    else
        restartLabel->show();
}

void GuiSettingsPage::applyChanges()
{
    Settings& settings = Settings::getInstance();
    settings.setAnimationEnabled(enableAnimationCheckbox->isChecked());
    settings.setGUILanguage(langCombo->itemData(langCombo->currentIndex()));
}

QGroupBox *GuiSettingsPage::buildAnimationGroup()
{
    QGroupBox *group = new QGroupBox(tr("Smoth animation"), this);
    QVBoxLayout* layout = new QVBoxLayout(group);
    enableAnimationCheckbox = new QCheckBox(tr("Enable animation"), group);

    layout->addWidget(enableAnimationCheckbox);
    return group;
}

QGroupBox *GuiSettingsPage::buildLanguageGroup()
{
    QGroupBox *group        = new QGroupBox(tr("Language"), this);
    QVBoxLayout* layout     = new QVBoxLayout(group);

    QFormLayout *formLayout = new QFormLayout;
    QLabel *langLabel       = new QLabel(tr("GUI language"), group);
    langCombo               = new QComboBox(group);
    langCombo->addItem(QIcon(":/icons/flag_great_britain.png"), "English", QVariant("en_GB"));
    langCombo->addItem(QIcon(":/icons/flag_germany.png"), "Deutsch", QVariant("de_DE"));

    restartLabel            = new QLabel(tr("You need to restart TOX."), this);
    restartLabel->setHidden(true);
    restartLabel->setDisabled(true);

    formLayout->setWidget(0, QFormLayout::LabelRole, langLabel);
    formLayout->setWidget(0, QFormLayout::FieldRole, langCombo);
    layout->addLayout(formLayout);
    layout->addWidget(restartLabel);
    return group;
}
