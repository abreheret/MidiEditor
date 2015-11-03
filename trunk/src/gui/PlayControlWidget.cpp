#include "PlayControlWidget.h"

#include <QPushButton>
#include <QGridLayout>
#include <QToolBar>

PlayControlWidget::PlayControlWidget(QWidget *parent) :  QWidget(parent) {

    setMinimumHeight(200);

    QGridLayout *l = new QGridLayout(this);
    setLayout(l);

    QToolBar *bar = new QToolBar(this);
    //bar->setIconSize(60, 60);
    //bar->addAction(backbackAction);
    //bar->addAction(backAction);
    //bar->addAction(playAction);
    //bar->addAction(pauseAction);
    //bar->addAction(playAction);
}
