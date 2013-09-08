#ifndef GUISETTINGSPAGE_H
#define GUISETTINGSPAGE_H

#include "abstractsettingspage.hpp"

class QGroupBox;
class QCheckBox;
class QComboBox;
class QLabel;

class GuiSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    GuiSettingsPage(QWidget *parent);

    void buildGui();
    void setGui();
    void applyChanges();

private slots:
	void showRestartInfo(int i);
    
private:
    QGroupBox* buildAnimationGroup();
    QGroupBox* buildLanguageGroup();

    // Aimation
    QCheckBox *enableAnimationCheckbox;

    // Language
    QComboBox *langCombo;
    QLabel    *restartLabel;
};

#endif // GUISETTINGSPAGE_H
