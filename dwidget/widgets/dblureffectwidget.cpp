#include "dblureffectwidget.h"
#include "private/dblureffectwidget_p.h"
#include "dplatformwindowhandle.h"
#include "dwindowmanagerhelper.h"

#include <QPainter>
#include <QBackingStore>
#include <QPaintEvent>
#include <QDebug>

#include <qpa/qplatformbackingstore.h>

#define MASK_COLOR_ALPHA_BLUR_MODE 102
#define MASK_COLOR_ALPHA_DEFAULT 204

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE

QMultiHash<QWidget*, const DBlurEffectWidget*> DBlurEffectWidgetPrivate::blurEffectWidgetHash;
QHash<const DBlurEffectWidget*, QWidget*> DBlurEffectWidgetPrivate::windowOfBlurEffectHash;

DBlurEffectWidgetPrivate::DBlurEffectWidgetPrivate(DBlurEffectWidget *qq)
    : DObjectPrivate(qq)
{

}

bool DBlurEffectWidgetPrivate::isBehindWindowBlendMode() const
{
    D_QC(DBlurEffectWidget);

    return blendMode == DBlurEffectWidget::BehindWindowBlend
            || q->isTopLevel();
}

void DBlurEffectWidgetPrivate::addToBlurEffectWidgetHash()
{
    D_QC(DBlurEffectWidget);

    QWidget *oldTopLevelWidget = windowOfBlurEffectHash.value(q);

    if (oldTopLevelWidget) {
        blurEffectWidgetHash.remove(oldTopLevelWidget, q);
        updateWindowBlurArea(oldTopLevelWidget);
    }

    QWidget *topLevelWidget = q->topLevelWidget();

    blurEffectWidgetHash.insertMulti(topLevelWidget, q);
    windowOfBlurEffectHash[q] = topLevelWidget;
    updateWindowBlurArea(topLevelWidget);
}

void DBlurEffectWidgetPrivate::removeFromBlurEffectWidgetHash()
{
    D_QC(DBlurEffectWidget);

    QWidget *topLevelWidget = windowOfBlurEffectHash.value(q);

    if (!topLevelWidget)
        return;

    blurEffectWidgetHash.remove(topLevelWidget, q);
    windowOfBlurEffectHash.remove(q);
    updateWindowBlurArea(topLevelWidget);
}

bool DBlurEffectWidgetPrivate::updateWindowBlurArea()
{
    D_QC(DBlurEffectWidget);

    QWidget *topLevelWidget = windowOfBlurEffectHash.value(q);

    return topLevelWidget && updateWindowBlurArea(topLevelWidget);
}

void DBlurEffectWidgetPrivate::setMaskColor(const QColor &color)
{
    maskColor = color;

    if (isBehindWindowBlendMode()) {
        maskColor.setAlpha(DWindowManagerHelper::instance()->hasBlurWindow() ? MASK_COLOR_ALPHA_BLUR_MODE : MASK_COLOR_ALPHA_DEFAULT);
    }
}

bool DBlurEffectWidgetPrivate::updateWindowBlurArea(QWidget *topLevelWidget)
{
    if (!topLevelWidget->isVisible())
        return false;

    QList<const DBlurEffectWidget*> blurEffectWidgetList = blurEffectWidgetHash.values(topLevelWidget);

    bool isExistMaskPath = false;

    Q_FOREACH (const DBlurEffectWidget *w, blurEffectWidgetList) {
        if (!w->d_func()->maskPath.isEmpty() && w->isVisible()) {
            isExistMaskPath = true;
            break;
        }
    }

    bool ok = false;

    if (isExistMaskPath) {
        QList<QPainterPath> pathList;

        Q_FOREACH (const DBlurEffectWidget *w, blurEffectWidgetList) {
            if (!w->isVisible())
                continue;

            QPainterPath p;
            QRect r = w->rect();

            r.moveTopLeft(w->mapTo(topLevelWidget, r.topLeft()));
            p.addRoundedRect(r, w->blurRectXRadius(), w->blurRectYRadius());

            if (!w->d_func()->maskPath.isEmpty()) {
                p &= w->d_func()->maskPath.translated(r.topLeft());
            }

            pathList << p;
        }

        ok = DPlatformWindowHandle::setWindowBlurAreaByWM(topLevelWidget, pathList);
    } else {
        QVector<DPlatformWindowHandle::WMBlurArea> areaList;

        areaList.reserve(blurEffectWidgetList.size());

        Q_FOREACH (const DBlurEffectWidget *w, blurEffectWidgetList) {
            if (!w->isVisible())
                continue;

            QRect r = w->rect();

            r.moveTopLeft(w->mapTo(topLevelWidget, r.topLeft()));

            areaList << dMakeWMBlurArea(r.x(), r.y(), r.width(), r.height(), w->blurRectXRadius(), w->blurRectYRadius());
        }

        ok = DPlatformWindowHandle::setWindowBlurAreaByWM(topLevelWidget, areaList);
    }

    if (blurEffectWidgetList.isEmpty()) {
        blurEffectWidgetHash.remove(topLevelWidget);
    }

    return ok;
}

DBlurEffectWidget::DBlurEffectWidget(QWidget *parent)
    : QWidget(parent)
    , DObject(*new DBlurEffectWidgetPrivate(this))
{
    if (!parent) {
        D_D(DBlurEffectWidget);

        d->addToBlurEffectWidgetHash();
    }

    QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::windowManagerChanged, this, [this] {
        D_D(DBlurEffectWidget);

        d->updateWindowBlurArea();
    });
    QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, this, [this] {
        D_D(DBlurEffectWidget);

        d->setMaskColor(d->maskColor);
    });
    QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, [this] {
        D_D(const DBlurEffectWidget);

        if (d->maskColorType != CustomColor)
            update();
    });
}

DBlurEffectWidget::~DBlurEffectWidget()
{
    D_D(DBlurEffectWidget);

    if (d->isBehindWindowBlendMode())
        d->removeFromBlurEffectWidgetHash();
}

int DBlurEffectWidget::radius() const
{
    D_DC(DBlurEffectWidget);

    return d->radius;
}

DBlurEffectWidget::BlurMode DBlurEffectWidget::mode() const
{
    D_DC(DBlurEffectWidget);

    return d->mode;
}

DBlurEffectWidget::BlendMode DBlurEffectWidget::blendMode() const
{
    D_DC(DBlurEffectWidget);

    return d->blendMode;
}

int DBlurEffectWidget::blurRectXRadius() const
{
    D_DC(DBlurEffectWidget);

    return d->blurRectXRadius;
}

int DBlurEffectWidget::blurRectYRadius() const
{
    D_DC(DBlurEffectWidget);

    return d->blurRectYRadius;
}

QColor DBlurEffectWidget::maskColor() const
{
    D_DC(DBlurEffectWidget);

    switch ((int)d->maskColorType) {
    case DarkColor: {
        if (!d->isBehindWindowBlendMode()) {
            return QColor(0, 0, 0, MASK_COLOR_ALPHA_BLUR_MODE);
        }

        if (DWindowManagerHelper::instance()->hasComposite()) {
            return QColor(0, 0, 0, DWindowManagerHelper::instance()->hasBlurWindow() ? MASK_COLOR_ALPHA_BLUR_MODE : MASK_COLOR_ALPHA_DEFAULT);
        } else {
            return QColor("#373F47");
        }
    }
    case LightColor: {
        if (!d->isBehindWindowBlendMode()) {
            return QColor(255, 255, 255, MASK_COLOR_ALPHA_BLUR_MODE);
        }

        if (DWindowManagerHelper::instance()->hasComposite()) {
            return QColor(255, 255, 255, DWindowManagerHelper::instance()->hasBlurWindow() ? MASK_COLOR_ALPHA_BLUR_MODE : MASK_COLOR_ALPHA_DEFAULT);
        } else {
            return QColor("#FCFCFC");
        }
    }
    }

    return d->maskColor;
}

void DBlurEffectWidget::setMaskPath(const QPainterPath &path)
{
    D_D(DBlurEffectWidget);

    if (d->maskPath == path)
        return;

    d->maskPath = path;

    update();
}

void DBlurEffectWidget::setRadius(int radius)
{
    D_D(DBlurEffectWidget);

    if (d->radius == radius)
        return;

    d->radius = radius;
    d->sourceImage = QImage();

    update();

    Q_EMIT radiusChanged(radius);
}

void DBlurEffectWidget::setMode(DBlurEffectWidget::BlurMode mode)
{
    D_D(DBlurEffectWidget);

    if (d->mode == mode)
        return;

    d->mode = mode;

    Q_EMIT modeChanged(mode);
}

void DBlurEffectWidget::setBlendMode(DBlurEffectWidget::BlendMode blendMode)
{
    D_D(DBlurEffectWidget);

    if (d->blendMode == blendMode)
        return;

    if (blendMode == BehindWindowBlend) {
        d->addToBlurEffectWidgetHash();
    } else {
        if (blendMode == InWindowBlend) {
            d->maskColor.setAlpha(MASK_COLOR_ALPHA_BLUR_MODE);
        }

        if (d->blendMode == BehindWindowBlend) {
            d->removeFromBlurEffectWidgetHash();
        }
    }

    d->blendMode = blendMode;

    update();

    Q_EMIT blendModeChanged(blendMode);
}

void DBlurEffectWidget::setBlurRectXRadius(int blurRectXRadius)
{
    D_D(DBlurEffectWidget);

    if (d->blurRectXRadius == blurRectXRadius)
        return;

    d->blurRectXRadius = blurRectXRadius;

    update();

    Q_EMIT blurRectXRadiusChanged(blurRectXRadius);
}

void DBlurEffectWidget::setBlurRectYRadius(int blurRectYRadius)
{
    D_D(DBlurEffectWidget);

    if (d->blurRectYRadius == blurRectYRadius)
        return;

    d->blurRectYRadius = blurRectYRadius;

    update();

    Q_EMIT blurRectYRadiusChanged(blurRectYRadius);
}

void DBlurEffectWidget::setMaskColor(QColor maskColor)
{
    D_D(DBlurEffectWidget);

    if (!maskColor.isValid()) {
        maskColor = Qt::transparent;
    }

    if (d->maskColor == maskColor)
        return;

    d->maskColorType = CustomColor;
    d->setMaskColor(maskColor);
    update();

    Q_EMIT maskColorChanged(maskColor);
}

void DBlurEffectWidget::setMaskColor(DBlurEffectWidget::MaskColorType type)
{
    D_D(DBlurEffectWidget);

    if (d->maskColorType == type)
        return;

    d->maskColorType = type;

    update();
}

DBlurEffectWidget::DBlurEffectWidget(DBlurEffectWidgetPrivate &dd, QWidget *parent)
    : QWidget(parent)
    , DObject(dd)
{

}

void DBlurEffectWidget::paintEvent(QPaintEvent *event)
{
    D_D(DBlurEffectWidget);

    QPainter pa(this);

    if (d->blurRectXRadius > 0 || d->blurRectYRadius > 0) {
        QPainterPath path;

        path.addRoundedRect(rect(), d->blurRectXRadius, d->blurRectYRadius);
        pa.setRenderHint(QPainter::Antialiasing);
        pa.setClipPath(path);
    }

    if (!d->maskPath.isEmpty()) {
        QPainterPath path = pa.clipPath();

        if (path.isEmpty()) {
            path = d->maskPath;
        } else {
            path &= d->maskPath;
        }

        pa.setClipPath(path);
    }

    if (d->isBehindWindowBlendMode()) {
        pa.setCompositionMode(QPainter::CompositionMode_Source);
    } else {
        int radius = d->radius;
        QPoint point_offset = mapTo(window(), QPoint(0, 0));
        const QRect &paintRect = event->rect();

        if (d->sourceImage.isNull()) {
            const QRect &tmp_rect = rect().translated(point_offset).adjusted(-radius, -radius, radius, radius);

            d->sourceImage = window()->backingStore()->handle()->toImage().copy(tmp_rect);
        } else {
            QPainter pa_image(&d->sourceImage);

            pa_image.setCompositionMode(QPainter::CompositionMode_Source);

            for (const QRect &rect : event->region().rects()) {
                pa_image.drawImage(rect.topLeft() + QPoint(radius, radius),
                                   window()->backingStore()->handle()->toImage().copy(rect.translated(point_offset)));
            }

            pa_image.end();
        }

        QImage image = d->sourceImage.copy(paintRect.adjusted(0, 0, 2 * radius, 2 * radius));

        QTransform old_transform = pa.transform();
        pa.translate(paintRect.topLeft() - QPoint(radius, radius));
        qt_blurImage(&pa, image, radius, false, false);
        pa.setTransform(old_transform);
    }

    pa.fillRect(event->rect(), maskColor());
}

void DBlurEffectWidget::moveEvent(QMoveEvent *event)
{
    D_D(DBlurEffectWidget);

    if (isTopLevel())
        return QWidget::moveEvent(event);

    if (d->blendMode == DBlurEffectWidget::InWindowBlend) {
        d->sourceImage = QImage();

        return QWidget::moveEvent(event);
    }

    d->updateWindowBlurArea();

    QWidget::moveEvent(event);
}

void DBlurEffectWidget::resizeEvent(QResizeEvent *event)
{
    D_D(DBlurEffectWidget);

    d->sourceImage = QImage();

    if (!d->isBehindWindowBlendMode())
        return QWidget::resizeEvent(event);

    d->updateWindowBlurArea();

    QWidget::resizeEvent(event);
}

void DBlurEffectWidget::showEvent(QShowEvent *event)
{
    D_D(DBlurEffectWidget);

    if (!d->isBehindWindowBlendMode())
        return QWidget::showEvent(event);

    d->addToBlurEffectWidgetHash();

    QWidget::showEvent(event);
}

void DBlurEffectWidget::hideEvent(QHideEvent *event)
{
    D_D(DBlurEffectWidget);

    if (!d->isBehindWindowBlendMode())
        return QWidget::hideEvent(event);

    d->removeFromBlurEffectWidgetHash();

    QWidget::hideEvent(event);
}

void DBlurEffectWidget::changeEvent(QEvent *event)
{
    D_D(DBlurEffectWidget);

    if (!d->isBehindWindowBlendMode())
        return QWidget::changeEvent(event);

    if (event->type() == QEvent::ParentAboutToChange) {
        d->removeFromBlurEffectWidgetHash();
    } else if (event->type() == QEvent::ParentChange) {
        d->addToBlurEffectWidgetHash();
    }

    QWidget::changeEvent(event);
}

DWIDGET_END_NAMESPACE
