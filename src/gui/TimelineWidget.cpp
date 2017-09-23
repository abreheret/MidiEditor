#include "TimelineWidget.h"

#include <QPainter>
#include "../midi/MidiPlayer.h"
#include "../protocol/Protocol.h"

TimelineWidget::TimelineWidget(QWidget *parent) : PaintWidget(parent) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFixedHeight(50);
	timeHeight = 50;
	file = 0;
	setRepaintOnMouseMove(false);
	setRepaintOnMousePress(false);
	setRepaintOnMouseRelease(false);
	setMouseTracking(true);
}

QSize TimelineWidget::sizeHint() const {
	return QSize(150, 50);
}
void TimelineWidget::mouseMoveEvent(QMouseEvent *event) {
	PaintWidget::mouseMoveEvent(event);
	if (!mouseInRect(relativeRect())) {
		return;
	}
	int xPos = qRound(mouseX);
	int oldXPos = qRound(mouseLastX);
	if (oldXPos < xPos) {
		matrixWidget->update(oldXPos -2, 0, qRound(mouseX) + 2,
								matrixWidget->height());
	} else {
		matrixWidget->update(xPos - 2, 0, oldXPos + 2,
								matrixWidget->height());
	}
}
void TimelineWidget::setMatrixWidget(MatrixWidget *widget) {
	matrixWidget = widget;
	file = matrixWidget->midiFile();
	matrixWidget->setTimelineWidget(this);
	setFixedWidth(matrixWidget->width());
	update();
}
qreal TimelineWidget::mousePosition() {
	if (mouseInRect(relativeRect())) {
		return mouseX;
	}
	return -1;
}
void TimelineWidget::setFile(MidiFile *midiFile) {
	file = midiFile;
	setMinimumWidth(file->maxTime() / 10);
	update();
}

void TimelineWidget::paintEvent(QPaintEvent *event) {
	if (!file || paintingActive() || !matrixWidget) {
		return;
	}
	QPainter painter(this);
	int updatemode = 0;
	if (!event->region().isNull() && !event->region().isEmpty()) {
		updatemode = 1;
		painter.setClipping(true);
		painter.setClipRegion(event->region());
	} else if (!event->rect().isNull() && !event->rect().isEmpty()) {
		updatemode = 2;
		painter.setClipping(true);
		painter.setClipRect(event->rect());
	} else {
		painter.setClipping(true);
		painter.setClipRect(relativeRect());
	}
	QFont font = painter.font();
	font.setPixelSize(12);
	painter.setFont(font);
	painter.setClipping(false);
	QPixmap pixmap;
	QString pixmapId = "TimelineWidget_" + QString::number(matrixWidget->scaleX, 'f', 2) + "_"
					   + file->protocol()->currentStepId();
	bool totalRepaint = !QPixmapCache::find(pixmapId, pixmap);
	if (totalRepaint) {
		pixmap = QPixmap(width(), height());
		if (pixmap.paintingActive()) {
			return;
		}
		QPainter pixpainter(&pixmap);
		if (MatrixWidget::antiAliasingEnabled) {
			pixpainter.setRenderHint(QPainter::Antialiasing);
		}
		pixpainter.fillRect(qRectF(0, 0, width(), timeHeight),
							QApplication::palette().window());

		pixpainter.setClipping(true);
		pixpainter.setClipRect(qRectF(0, 0, width() - 2,
									  height()));

		pixpainter.setPen(Qt::darkGray);
		pixpainter.setBrush(Qt::white);
		pixpainter.drawRect(qRectF(0, 2, width() - 1, timeHeight - 2));
		pixpainter.setPen(Qt::black);

		pixpainter.fillRect(qRectF(0, timeHeight - 3, width(), 3),
							QApplication::palette().window());

		// paint time (ms)
		int numbers = (parentWidget()->width()) / 80;
		if (numbers > 0) {
			int step = file->endTick() / numbers;
			int realstep = 1;
			int nextfak = 2;
			int tenfak = 1;
			while (realstep <= step) {
				realstep = nextfak * tenfak;
				if (nextfak == 1) {
					nextfak++;
					continue;
				}
				if (nextfak == 2) {
					nextfak = 5;
					continue;
				}
				if (nextfak == 5) {
					nextfak = 1;
					tenfak *= 10;
				}
			}
			int startNumber = 0;
			startNumber *= realstep;
			if (startNumber < file->endTick()) {
				startNumber += realstep;
			}
			pixpainter.setPen(Qt::gray);
			while (startNumber < file->endTick()) {
				int pos = matrixWidget->xPosOfMs(matrixWidget->msOfTick(startNumber));
				//((startNumber)*(file->endTick()))/width();
				QString text = "";
				int hours = startNumber / (60000 * 60);
				int remaining = startNumber - (60000 * 60) * hours;
				int minutes = remaining / (60000);
				remaining = remaining - minutes * 60000;
				int seconds = remaining / 1000;
				int ms = remaining - 1000 * seconds;


				text += QString::number(hours) + ":";
				text += QString("%1:").arg(minutes, 2, 10, QChar('0'));
				text += QString("%1").arg(seconds, 2, 10, QChar('0'));
				text += QString(".%1").arg(ms / 10, 2, 10, QChar('0'));
				int textlength = QFontMetricsF(pixpainter.font()).width(text);
				if (startNumber > 0) {
					pixpainter.drawText(qPointF(pos - textlength / 2, timeHeight / 2 - 6), text);
				}
				pixpainter.drawLine(qLineF(pos, timeHeight / 2 - 1, pos, timeHeight));
				startNumber += realstep;
			}
			currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
			int measure = file->measure(0, file->endTick(), &currentTimeSignatureEvents);

			TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
			int i = 0;
			if (!currentEvent) {
				return;
			}
			int tick = currentEvent->midiTime();
			while (tick + currentEvent->ticksPerMeasure() <= 0) {
				tick += currentEvent->ticksPerMeasure();
			}
			while (tick < file->endTick()) {
				qreal xfrom = matrixWidget->xPosOfMs(matrixWidget->msOfTick(tick));
				matrixWidget->currentDivs.append(QPair<qreal, int>(xfrom, tick));
				measure++;
				tick += currentEvent->ticksPerMeasure();
				if (i < currentTimeSignatureEvents->length() - 1) {
					if (currentTimeSignatureEvents->at(i + 1)->midiTime() <= tick) {
						currentEvent = currentTimeSignatureEvents->at(i + 1);
						tick = currentEvent->midiTime();
						i++;
					}
				}
				int xto = matrixWidget->xPosOfMs(matrixWidget->msOfTick(tick));
				pixpainter.setBrush(Qt::lightGray);
				pixpainter.setPen(Qt::NoPen);
				pixpainter.drawRoundedRect(qRectF(xfrom + 2, timeHeight / 2 + 4,
												  xto - xfrom - 4, timeHeight / 2 - 10), 5, 5);
				if (tick > 0) {
					pixpainter.setPen(Qt::gray);
					pixpainter.drawLine(qLineF(xfrom, timeHeight / 2, xfrom, height()));
					QString text = "Measure " + QString::number(measure - 1);
					qreal textlength = QFontMetricsF(pixpainter.font()).width(text);
					if (textlength > xto - xfrom) {
						text = QString::number(measure - 1);
						textlength = QFontMetricsF(pixpainter.font()).width(text);
					}
					qreal pos = (xfrom + xto) / 2;
					pixpainter.setPen(Qt::white);
					pixpainter.drawText(qPointF(pos - textlength / 2, timeHeight - 9), text);
				}
			}
		}
		pixpainter.end();
		QPixmapCache::insert(pixmapId, pixmap);
		QPalette palette = this->palette();
		palette.setBrush(this->backgroundRole(), pixmap);
		setPalette(palette);
	}
	if (!QApplication::arguments().contains("--no-antialiasing")
			&& MatrixWidget::antiAliasingEnabled) {
		painter.setRenderHint(QPainter::Antialiasing);
	}
	/*switch (updatemode) {
		case 1: {
			QPainter::PixmapFragment frags[event->region().rectCount()];
			QVector<QRect> rects = event->region().rects();
			for (int i = 0; i < event->region().rectCount(); i++) {
				QRect rect = rects.at(i);
				frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
			}
			painter.drawPixmapFragments(frags, event->region().rectCount(), pixmap);
			break;
		}
		case 2: {
			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
												event->rect().center(), event->rect());
			painter.drawPixmapFragments(&frag, 1, pixmap);
			break;
		}
		default:
			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
												relativeRect().center(), relativeRect());
			painter.drawPixmapFragments(&frag, 1, pixmap);
	}*/
	// paint the cursorTick of file
	if (file->cursorTick() >= 0 &&
			file->cursorTick() <= file->endTick()) {
		painter.setPen(Qt::darkGray);
		int x = matrixWidget->xPosOfMs(matrixWidget->msOfTick(file->cursorTick()));
		if (relativeRect().contains(x, 0)) {
			painter.drawLine(x, 0, x, height());
			QPointF points[3] = {
				qPointF(x - 8, timeHeight / 2 + 2),
				qPointF(x + 8, timeHeight / 2 + 2),
				qPointF(x, timeHeight - 2),
			};

			painter.setBrush(QBrush(QColor(194, 230, 255), Qt::SolidPattern));

			painter.drawPolygon(points, 3);
			painter.setPen(Qt::gray);
		}
	}
	// paint the pauseTick of file if >= 0
	if (!MidiPlayer::instance()->isPlaying() && file->pauseTick() >= 0 &&
			file->pauseTick() <= file->endTick()) {
		int x = matrixWidget->xPosOfMs(matrixWidget->msOfTick(file->pauseTick()));
		if (relativeRect().contains(x, 0)) {
			QPointF points[3] = {
				qPointF(x - 8, timeHeight / 2 + 2),
				qPointF(x + 8, timeHeight / 2 + 2),
				qPointF(x, timeHeight - 2),
			};
			painter.setBrush(QBrush(Qt::gray, Qt::SolidPattern));
			painter.drawPolygon(points, 3);
		}
	}
}
void TimelineWidget::mouseDoubleClickEvent(QMouseEvent *event) {
	if (!matrixWidget) {
		return;
	}
	if (mouseInRect(relativeRect())) {
		int tick = file->tick(matrixWidget->msOfXPos(mouseX));
		file->setCursorTick(tick);
		update();
		matrixWidget->update();
	}
}
