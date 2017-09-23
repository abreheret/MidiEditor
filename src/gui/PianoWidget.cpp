#include "PianoWidget.h"

#include <QPainter>
#include "../tool/Selection.h"

/*
 * FIXME:
 * - This is literally a static image. There is no
 *   interaction whatsoever.
 */
PianoWidget::PianoWidget(QWidget *parent) : PaintWidget(parent) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFixedWidth(110);
	setMinimumHeight(1);
	setRepaintOnMouseMove(false);
	setRepaintOnMouseMove(false);
	setRepaintOnMouseRelease(false);
	setMouseTracking(true);
}
void PianoWidget::setMatrixWidget(MatrixWidget *widget) {
	matrixWidget = widget;
	setMinimumHeight(matrixWidget->height());
	matrixWidget->setPianoWidget(this);
	update();
}
void PianoWidget::setFile(MidiFile *file) {
	Q_UNUSED(file)
	update();
}
QSize PianoWidget::sizeHint() const {
	return QSize(110, 150);
}
void PianoWidget::paintEvent(QPaintEvent *event) {

	if (paintingActive() || !matrixWidget || !matrixWidget->midiFile()) {
		return;
	}
	QPainter painter(this);
	QFont font = painter.font();
	font.setPixelSize(12);
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
	QPixmap pianoPixmap;
	if (!QPixmapCache::find("PianoWidget_" + QString::number(matrixWidget->scaleY, 'f', 2), pianoPixmap)) {
		matrixWidget->pianoKeys.clear();
		setFixedHeight(matrixWidget->height());

		pianoPixmap = QPixmap(110, height());

		QPainter pixpainter(&pianoPixmap);
		if (!QApplication::arguments().contains("--no-antialiasing")
				&& MatrixWidget::antiAliasingEnabled) {
			pixpainter.setRenderHint(QPainter::Antialiasing);
		}
		pixpainter.fillRect(qRectF(0, 0, 110, height()), QApplication::palette().window());
		qreal numLines = matrixWidget->endLineY - matrixWidget->startLineY;
		if (qFuzzyIsNull(numLines)) {
			qWarning("Invalid numLines. ");
			pixpainter.end();
			painter.end();
			return;
		}
		int pianoKeys = qRound(numLines);
		if (matrixWidget->endLineY > 127) {
			pianoKeys -= (matrixWidget->startLineY - 127);
		}
		if (pianoKeys > 0) {
			pixpainter.fillRect(qRectF(0, 0, 100,
										pianoKeys * -1 * matrixWidget->lineHeight()), Qt::white);
		}
		for (double i = matrixWidget->startLineY; i <= matrixWidget->endLineY; i++) {
			int startLine = matrixWidget->yPosOfLine(qRound(i));
			if (i >= 0 && i <= 127) {
				paintPianoKey(&pixpainter, 127 - qFloor(i), 0, startLine,
							  110, matrixWidget->lineHeight());
			} else {
				QString text = "";
				switch (qFloor(i)) {
					case MidiEvent::CONTROLLER_LINE: {
						text = "Control Change";
						break;
					}
					case MidiEvent::TEMPO_CHANGE_EVENT_LINE: {
						text = "Tempo Change";
						break;
					}
					case MidiEvent::TIME_SIGNATURE_EVENT_LINE: {
						text = "Time Signature";
						break;
					}
					case MidiEvent::KEY_SIGNATURE_EVENT_LINE: {
						text = "Key Signature.";
						break;
					}
					case MidiEvent::PROG_CHANGE_LINE: {
						text = "Program Change";
						break;
					}
					case MidiEvent::KEY_PRESSURE_LINE: {
						text = "Key Pressure";
						break;
					}
					case MidiEvent::CHANNEL_PRESSURE_LINE: {
						text = "Channel Pressure";
						break;
					}
					case MidiEvent::TEXT_EVENT_LINE: {
						text = "Text";
						break;
					}
					case MidiEvent::PITCH_BEND_LINE: {
						text = "Pitch Bend";
						break;
					}
					case MidiEvent::SYSEX_LINE: {
						text = "System Exclusive";
						break;
					}
					case MidiEvent::UNKNOWN_LINE: {
						text = "(Unknown)";
						break;
					}
				}
				pixpainter.setPen(Qt::darkGray);
				QFont font = pixpainter.font();
				font.setPixelSize(9);
				pixpainter.setFont(font);
				qreal textlength = QFontMetricsF(font).width(text);
				qreal textHeight = QFontMetricsF(font).height();
				// Don't draw text if zoomed too far out.
				if (textHeight < (matrixWidget->lineHeight() + 1)) {
					pixpainter.drawText(qPointF(95 - textlength, startLine +
										   matrixWidget->lineHeight() - 1), text);
				}
			}
		}
		pixpainter.end();
		QPixmapCache::insert("PianoWidget_" + QString::number(matrixWidget->scaleY, 'f', 2), pianoPixmap);
		QPalette palette = this->palette();
		palette.setBrush(backgroundRole(), pianoPixmap);
		setPalette(palette);
	}
	/*switch (updatemode) {
		case 1: {
			QPainter::PixmapFragment frags[event->region().rectCount()];
			QVector<QRect> rects = event->region().rects();
			for (int i = 0; i < event->region().rectCount(); i++) {
				QRect rect = rects.at(i);
				frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
			}
			painter.drawPixmapFragments(frags, event->region().rectCount(), pianoPixmap);
			break;
		}
		case 2: {
			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
												event->rect().center(), event->rect());
			painter.drawPixmapFragments(&frag, 1, pianoPixmap);
			break;
		}
		default:
			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
												relativeRect().center(), relativeRect());
			painter.drawPixmapFragments(&frag, 1, pianoPixmap);
	}*/
}

void PianoWidget::paintPianoKey(QPainter *painter, int number, qreal x,
								qreal y,
								qreal width, qreal height) {
	int borderRight = 10;
	width = width - borderRight;
	if (number >= 0 && number <= 127) {

		double scaleHeightBlack = 0.5;
		double scaleWidthBlack = 0.6;

		PianoKeyShape shape;
		QString name = "";

		switch (number % 12) {
			case C: {
				shape = WhiteBelowBlack;
				int i = number / 12;
				//if(i<4){
				//	name="C";{
				//		for(int j = 0; j<3-i; j++){
				//			name+="'";
				//		}
				//	}
				//} else {
				//	name = "c";
				//	for(int j = 0; j<i-4; j++){
				//		name+="'";
				//	}
				//}
				name = "C" + QString::number(i - 1);
				break;
			}
			case C_Sharp: {
				shape = Black;
				break;
			}
			case D: {
				shape = WhiteBetweenBlack;
				break;
			}
			case D_Sharp: {
				shape = Black;
				break;
			}
			case E: {
				shape = WhiteAboveBlack;
				break;
			}
			case F: {
				shape = WhiteBelowBlack;
				break;
			}
			case F_Sharp: {
				shape = Black;
				break;
			}
			// G
			case G: {
				shape = WhiteBetweenBlack;
				break;
			}
			// g#
			case G_Sharp: {
				shape = Black;
				break;
			}
			// A
			case A: {
				shape = WhiteBetweenBlack;
				break;
			}
			// A#
			case A_Sharp: {
				shape = Black;
				break;
			}
			// B
			case B: {
				shape = WhiteAboveBlack;
				break;
			}
		}

		bool selected = mouseY >= y && mouseY <= y + height && mouseX > 0
						&& mouseOver;
		foreach (MidiEvent *event, Selection::instance()->selectedEvents()) {
			if (event->line() == qFloor(127 - number)) {
				selected = true;
				break;
			}
		}

		QPolygonF keyPolygon;

		bool inRect = false;

		if (shape == Black) {
			painter->drawLine(qLineF(x, y + height / 2, x + width, y + height / 2));
			y += (height - height * scaleHeightBlack) / 2;
			QRectF playerRect;
			playerRect.setX(x);
			playerRect.setY(y);
			playerRect.setWidth(width * scaleWidthBlack);
			playerRect.setHeight(height * scaleHeightBlack + 0.5);
			QColor c = Qt::black;
			if (mouseInWidget(matrixWidget)) {
				c = QColor(200, 200, 200);
				inRect = true;
			}
			painter->fillRect(qRectF(playerRect), c);

			keyPolygon.append(qPointF(x, y));
			keyPolygon.append(qPointF(x, y + height * scaleHeightBlack));
			keyPolygon.append(qPointF(x + width * scaleWidthBlack,
									  y + height * scaleHeightBlack));
			keyPolygon.append(qPointF(x + width * scaleWidthBlack, y));
			matrixWidget->pianoKeys.insert(number, qRectF(playerRect));

		} else {

			if (shape == WhiteAboveBlack) {
				keyPolygon.append(qPointF(x, y));
				keyPolygon.append(qPointF(x + width, y));
			} else {
				keyPolygon.append(qPointF(x, y - height * scaleHeightBlack / 2));
				keyPolygon.append(qPointF(x + width * scaleWidthBlack,
										  y - height * scaleHeightBlack / 2));
				keyPolygon.append(qPointF(x + width * scaleWidthBlack,
										  y - height * scaleHeightBlack));
				keyPolygon.append(qPointF(x + width, y - height * scaleHeightBlack));
			}
			if (shape == WhiteBelowBlack) {
				painter->drawLine(qLineF(x, y + height, x + width, y + height));
				keyPolygon.append(qPointF(x + width, y + height));
				keyPolygon.append(qPointF(x, y + height));
			} else {
				keyPolygon.append(qPointF(x + width, y + height + height * scaleHeightBlack));
				keyPolygon.append(qPointF(x + width * scaleWidthBlack,
										  y + height + height * scaleHeightBlack));
				keyPolygon.append(qPointF(x + width * scaleWidthBlack,
										  y + height + height * scaleHeightBlack / 2));
				keyPolygon.append(qPointF(x, y + height + height * scaleHeightBlack / 2));
			}
			inRect = mouseInRect(x, y, width, height);
			matrixWidget->pianoKeys.insert(number, qRectF(x, y, width, height));
		}

		if (shape == Black) {
			if (inRect) {
				painter->setBrush(Qt::lightGray);
			} else if (selected) {
				painter->setBrush(Qt::darkGray);
			} else {
				painter->setBrush(Qt::black);
			}
		} else {
			if (inRect) {
				painter->setBrush(Qt::darkGray);
			} else if (selected) {
				painter->setBrush(Qt::lightGray);
			} else {
				painter->setBrush(Qt::white);
			}
		}
		painter->setPen(Qt::darkGray);
		painter->drawPolygon(qPolygonF(keyPolygon), Qt::OddEvenFill);


		if (name != "") {
			painter->setPen(Qt::gray);
			int textlength = QFontMetrics(painter->font()).width(name);
			painter->drawText(qPointF(x + width - textlength - 2, y + height - 1), name);
			painter->setPen(Qt::black);
		}
		if (inRect && enabled) {
			// mark the current Line
			QColor lineColor = QColor(0, 0, 100, 40);
			painter->fillRect(qRectF(x + width + borderRight, matrixWidget->yPosOfLine(127 - number),
									 this->width() - x - width - borderRight, height), lineColor);
		}
	}
}
