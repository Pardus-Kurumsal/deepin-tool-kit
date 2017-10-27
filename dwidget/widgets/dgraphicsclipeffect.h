#ifndef DGRAPHICSCLIPEFFECT_H
#define DGRAPHICSCLIPEFFECT_H

#include "dwidget_global.h"
#include "dobject.h"

#include <QGraphicsEffect>

DWIDGET_BEGIN_NAMESPACE

class DGraphicsClipEffectPrivate;
class DGraphicsClipEffect : public QGraphicsEffect, public DObject
{
    Q_OBJECT

    Q_PROPERTY(QMargins margins READ margins WRITE setMargins NOTIFY marginsChanged)
    Q_PROPERTY(QPainterPath clipPath READ clipPath WRITE setClipPath NOTIFY clipPathChanged)

public:
    explicit DGraphicsClipEffect(QObject *parent = Q_NULLPTR);

    QMargins margins() const;
    QPainterPath clipPath() const;

public Q_SLOTS:
    void setMargins(const QMargins &margins);
    void setClipPath(const QPainterPath &clipPath);

Q_SIGNALS:
    void marginsChanged(QMargins margins);
    void clipPathChanged(QPainterPath clipPath);

protected:
    void draw(QPainter *painter) Q_DECL_OVERRIDE;

private:
    D_DECLARE_PRIVATE(DGraphicsClipEffect)
};

DWIDGET_END_NAMESPACE

#endif // DGRAPHICSCLIPEFFECT_H
