#ifndef GUISETTINGSWIDGET_H
#define GUISETTINGSWIDGET_H

#include <QObject>
#include "SettingsWidget.h"

class QWidget;
class QCheckBox;
class QSettings;

class GuiSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    GuiSettingsWidget(QSettings *settings, QWidget *parent = 0);

public slots:
    void setAntiAliasing(bool enable);
    void setSelectAndMove(bool enable);
    void setVelocityDragging(bool enable);

private:
    QSettings *_settings;
    QCheckBox *_selectAndMove;
    QCheckBox *_antiAliasingBox;
    QCheckBox *_velocityDraggingBox;
};

#endif // GUISETTINGSWIDGET_H
