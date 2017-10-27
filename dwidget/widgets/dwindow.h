#ifndef DWINDOW_H
#define DWINDOW_H

#include <QWidget>
#include <dobject.h>
#include <dwidget_global.h>

#include "dwidget.h"

class QMenu;

DWIDGET_BEGIN_NAMESPACE

class DWindowPrivate;

class LIBDTKWIDGETSHARED_EXPORT DWindow : public DWidget
{
    Q_OBJECT
public:
    explicit DWindow(QWidget *parent = 0);

    void setContentLayout(QLayout *l);
    void setContentWidget(QWidget *w);

    void setParent(QWidget *parent);
    QWidget *parentWidget() const;

protected Q_SLOTS:

private:
    D_DECLARE_PRIVATE(DWindow)
    Q_DISABLE_COPY(DWindow)
};

DWIDGET_END_NAMESPACE

#endif // DWINDOW_H
