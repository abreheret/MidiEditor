#include "MiscWidget.h"
#include "MatrixWidget.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiTrack.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../tool/EventTool.h"
#include "../tool/SelectTool.h"
#include "../tool/NewNoteTool.h"
#include "../protocol/Protocol.h"
#include "../tool/Selection.h"

#include <QPixmapCache>

#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/PitchBendEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/ChannelPressureEvent.h"

#define LEFT_BORDER_MATRIX_WIDGET 0
#define WIDTH 7

/*
 * FIXME:
 * - Kill all the forced repaint hacks
 * - Updates aren't syncing
 *   up with the protocol
 * - New notes often set to tick 0
 * - Divs aren't being painted
 *   the first time
 * - Fix measurements
 * TODO:
 * - Set everything that doesn't need to be
 *   repainted to the background, and what does
 *   to the foreground.
 * - Color the channel lines?
 * - Smooth creation of lines; i.e. you click and
 *   drag for instant feedback.
 * - Find the proper update() time
 * - update() is regional
 */
MiscWidget::MiscWidget(MatrixWidget *mw,
					   QWidget *parent) : PaintWidget(parent) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	// FIXME: hack
	setRepaintOnMouseMove(true);
	setRepaintOnMousePress(true);
	setRepaintOnMouseRelease(true);
	matrixWidget = mw;
	edit_mode = SINGLE_MODE;
	mode = VelocityEditor;
	channel = 0;
	controller = 0;
	file = 0;
	dragY = 0;
	isDrawingFreehand = false;
	isDrawingLine = false;
	resetState();
	computeMinMax();
	setFixedWidth(matrixWidget->width());
	connect(matrixWidget, SIGNAL(objectListChanged()), this, SLOT(redraw()));
	_dummyTool = new SelectTool(SELECTION_TYPE_SINGLE);
	setFocusPolicy(Qt::ClickFocus);
}

void MiscWidget::redraw() {
	update();
}

void MiscWidget::setFile(MidiFile *midiFile) {
	file = midiFile;
	connect(file->protocol(), SIGNAL(actionFinished()), this, SLOT(redraw()));
	connect(Selection::instance(), SIGNAL(selectionChanged()), this, SLOT(redraw()));
	update();
}
QString MiscWidget::modeToString(int mode) {
	switch (mode) {
		case VelocityEditor:
			return "Velocity";
		case ControlEditor:
			return "Control Change";
		case PitchBendEditor:
			return "Pitch Bend";
		case KeyPressureEditor:
			return "Key Pressure";
		case ChannelPressureEditor:
			return "Channel Pressure";
	}
	return "";
}

void MiscWidget::setMode(int mode) {
	this->mode = mode;
	resetState();
	computeMinMax();
}

void MiscWidget::setEditMode(int mode) {
	this->edit_mode = mode;
	resetState();
	computeMinMax();
}

void MiscWidget::setChannel(int channel) {
	this->channel = channel;
	resetState();
	computeMinMax();
}

void MiscWidget::setControl(int ctrl) {
	this->controller = ctrl;
	resetState();
	computeMinMax();
}

void MiscWidget::paintEvent(QPaintEvent *event) {
	if (!matrixWidget || !file || paintingActive() || height() < 10) {
		return;
	}

	if (width() != matrixWidget->width()) {
		setFixedWidth(matrixWidget->width());
	}

	// draw background
	QPainter painter(this);
	if (MatrixWidget::antiAliasingEnabled) {
		painter.setRenderHint(QPainter::Antialiasing);
	}
	QFont f = painter.font();
	f.setPixelSize(9);
	painter.setFont(f);
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
	// divs
	QPixmap notes;
	QString notesId = "MiscWidget_" + QString::number(height()) + "_" +
					  file->protocol()->currentStepId()
					  + "_" + QString::number(mode);

	// FIXME: Hacks
	bool hackyMouseFullUpdate = mouseInRect(relativeRect());
	bool hackyInvalidate = !QPixmapCache::find(notesId, notes);
	if (hackyInvalidate || hackyMouseFullUpdate || dragging) {
		QPixmap background;
		if (!QPixmapCache::find("MiscWidget_" + QString::number(height()),
								background)) {
			background = QPixmap(1, height());
			background.fill(Qt::transparent);
			QPainter pixpainter(&background);
			QColor c(234, 246, 255);
			if (MatrixWidget::antiAliasingEnabled) {
				pixpainter.setRenderHint(QPainter::Antialiasing);
			}
			pixpainter.setPen(c);
			pixpainter.setBrush(c);
			pixpainter.drawRect(qRectF(0, 0, 0, relativeRect().height() ));

			pixpainter.setPen(QColor(194, 230, 255));
			for (int i = 0; i < 8; i++) {
				pixpainter.drawLine(qLineF(0, (i * height()) / 8, 1, (i * height()) / 8));
			}
			pixpainter.end();
			QPixmapCache::insert("MiscWidget_" + QString::number(height()), background);
		}

		notes = QPixmap(width(), height());
		notes.fill(Qt::transparent);
		QPainter pixpainter(&notes);
		if (MatrixWidget::antiAliasingEnabled) {
			pixpainter.setRenderHint(QPainter::Antialiasing);
		}
		pixpainter.fillRect(0, 0, width(), height(), QBrush(background));
		pixpainter.setBrush(QColor(234, 246, 255));
		pixpainter.setPen(QColor(194, 230, 255));
		typedef QPair<qreal, int> TMPPair;
		foreach (TMPPair p, matrixWidget->divs()) {
			pixpainter.drawLine(qLineF(p.first - LEFT_BORDER_MATRIX_WIDGET, 0,
									   p.first - LEFT_BORDER_MATRIX_WIDGET, height()));
		}

		// draw contents
		if (mode == VelocityEditor) {

			QList<MidiEvent *> *list = matrixWidget->velocityEvents();
			foreach (MidiEvent *event, *list) {

				if (!file->channel(event->channel())->visible()) {
					continue;
				}

				if (event->track()->hidden()) {
					continue;
				}

				QColor c = file->channel(event->channel())->color();
				if (!matrixWidget->colorsByChannel()) {
					c = *event->track()->color();
				}

				int velocity = 0;
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
				if (noteOn) {
					velocity = noteOn->velocity();

					if (velocity > 0) {
						int h = (height() * velocity) / 128;
						pixpainter.setBrush(c);
						pixpainter.setPen(Qt::lightGray);
						pixpainter.drawRoundedRect(qRectF(event->x() - LEFT_BORDER_MATRIX_WIDGET,
														  height() - h, WIDTH, h), 1, 1);
					}
				}
			}

			// paint selected events above all others
			EventTool *t = qobject_cast<EventTool *>(Tool::currentTool());
			if (t && t->showsSelection()) {
				foreach (MidiEvent *event, Selection::instance()->selectedEvents()) {

					if (!file->channel(event->channel())->visible()) {
						continue;
					}

					if (event->track()->hidden()) {
						continue;
					}

					int velocity = 0;
					double velocityMultiplier = 1.0;
					NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);

					if (noteOn && noteOn->midiTime() >= matrixWidget->minVisibleMidiTime()
							&& noteOn->midiTime() <= matrixWidget->maxVisibleMidiTime()) {
						velocity = noteOn->velocity();
						if (aboveEvent) {
							velocityMultiplier = velocity / aboveEvent->velocity();
						}


						if (velocity > 0) {
							int h = (height() * velocity) / 128;
							if (edit_mode == SINGLE_MODE && dragging) {
								h = velocityMultiplier * (h + (dragY - mouseY));
							}
							pixpainter.setBrush(Qt::darkBlue);
							pixpainter.setPen(Qt::lightGray);
							pixpainter.drawRoundedRect(qRectF(event->x() - LEFT_BORDER_MATRIX_WIDGET,
															  height() - h, WIDTH, h), 1, 1);
						}
					}
				}
			}
		}


		// draw content track
		if (mode > VelocityEditor) {

			QColor c = file->channel(0)->color();
			QPen pen(c);
			pen.setWidth(3);
			pixpainter.setPen(pen);

			QPen circlePen(Qt::darkGray);
			circlePen.setWidth(1);

			QList<MidiEvent *> accordingEvents;
			QList<QPair<int, int> > track = getTrack(&accordingEvents);

			qreal xOld;
			qreal yOld;

			for (int i = 0; i < track.size(); i++) {

				qreal xPix = track.at(i).first;
				qreal yPix = height() - ((double)track.at(i).second / (double)_max) * height();
				if (edit_mode == SINGLE_MODE) {
					if (i == trackIndex) {
						if (dragging) {
							yPix = yPix + mouseY - dragY;
						}
					}
				}
				if (i > 0) {
					pixpainter.drawLine(qLineF(xOld, yOld, xPix, yOld));
					pixpainter.drawLine(qLineF(xPix, yOld, xPix, yPix));
				}
				xOld = xPix;
				yOld = yPix;
			}
			pixpainter.drawLine(qLineF(xOld, yOld, width(), yOld));


			for (int i = 0; i < track.size(); i++) {

				qreal xPix = track.at(i).first;
				qreal yPix = height() - ((double)track.at(i).second / (double)_max) * height();
				if (edit_mode == SINGLE_MODE) {
					if (i == trackIndex) {
						if (dragging) {
							yPix = yPix + mouseY - dragY;
						}
					}
				}

				if (edit_mode == SINGLE_MODE && (dragging || mouseOver)) {

					if (accordingEvents.at(i)
							&& Selection::instance()->selectedEvents().contains(accordingEvents.at(i))) {
						pixpainter.setBrush(Qt::darkBlue);
					}
					pixpainter.setPen(circlePen);
					if (i == trackIndex) {
						pixpainter.setBrush(Qt::gray);
					}
					pixpainter.drawEllipse(qRectF(xPix - 4, yPix - 4, 8, 8));
					pixpainter.setPen(pen);
					pixpainter.setBrush(Qt::NoBrush);
				}
			}
		}


		// draw freehand track
		if (edit_mode == MOUSE_MODE && isDrawingFreehand && freeHandCurve.size() > 0) {

			qreal xOld;
			qreal yOld;

			QPen pen(Qt::darkBlue);
			pen.setWidth(3);
			pixpainter.setPen(pen);

			for (int i = 0; i < freeHandCurve.size(); i++) {
				qreal xPix = freeHandCurve.at(i).first;
				qreal yPix = freeHandCurve.at(i).second;
				if (i > 0) {
					pixpainter.drawLine(qLineF(xOld, yOld, xPix, yPix));
				}
				xOld = xPix;
				yOld = yPix;
			}
		}

		// draw line
		if (edit_mode == LINE_MODE && isDrawingLine) {

			QPen pen(Qt::darkBlue);
			pen.setWidth(3);
			pixpainter.setPen(pen);

			pixpainter.drawLine(qLineF(lineX, lineY, mouseX, mouseY));
		}
		pixpainter.end();
		// FIXME: More hacks
		if (hackyInvalidate) {
			QPixmapCache::insert(notesId, notes);
		}
			QPalette palette = this->palette();
			palette.setBrush(backgroundRole(), QBrush(notes));
			setPalette(palette);
		/*} else {
			switch (updatemode) {
				case 1: {
					QPainter::PixmapFragment frags[event->region().rectCount()];
					QVector<QRect> rects = event->region().rects();
					for (int i = 0; i < event->region().rectCount(); i++) {
						QRect rect = rects.at(i);
						frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
					}
					painter.drawPixmapFragments(frags, event->region().rectCount(), notes);
					break;
				}
				case 2: {
					QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
														event->rect().center(), event->rect());
					painter.drawPixmapFragments(&frag, 1, notes);
					break;
				}
				default:
					QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
														relativeRect().center(), relativeRect());
					painter.drawPixmapFragments(&frag, 1, notes);
			}
		}*/
	}

}

void MiscWidget::mouseMoveEvent(QMouseEvent *event) {
	if (edit_mode == SINGLE_MODE) {
		if (mode == VelocityEditor) {
			bool above = dragging;
			if (!above) {
				QList<MidiEvent *> *list = matrixWidget->velocityEvents();
				foreach (MidiEvent *event, *list) {

					if (!event->file()->channel(event->channel())->visible()) {
						continue;
					}

					if (event->track()->hidden()) {
						continue;
					}

					int velocity = 0;
					NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
					if (noteOn) {
						velocity = noteOn->velocity();

						if (velocity > 0) {
							int h = (height() * velocity) / 128;
							if (!dragging
									&& mouseInRect(qRectF(event->x() - LEFT_BORDER_MATRIX_WIDGET, height() - h - 5, WIDTH,
												   10))) {
								above = true;
								aboveEvent = noteOn;
								break;
							}
						}
					}
				}
			}
			if (above) {
				setCursor(Qt::SizeVerCursor);
			} else {
				setCursor(Qt::ArrowCursor);
			}

		} else {

			//other modes
			if (!dragging) {
				trackIndex = -1;
				QList<QPair<int, int> > track = getTrack();
				for (int i = 0; i < track.size(); i++) {

					qreal xPix = track.at(i).first;
					qreal yPix = height() - (qreal(track.at(i).second) / qreal(_max)) * height();

					if (mouseInRect(xPix - 4, yPix - 4, 8, 8)) {
						trackIndex = i;
						//setCursor(Qt::SizeVerCursor);
						setCursor(Qt::ArrowCursor);
						break;
					}
				}

				if (trackIndex == -1) {
					setCursor(Qt::ArrowCursor);
				}
			} else {
				setCursor(Qt::SizeVerCursor);
			}
		}
	}
	if (edit_mode == MOUSE_MODE) {
		if (isDrawingFreehand) {
			bool ok = true;
			for (int i = 0; i < freeHandCurve.size(); i++) {
				if (freeHandCurve.at(i).first >= qPointF(event->localPos()).x()) {
					ok = false;
					break;
				}
			}
			if (ok) {
				freeHandCurve.append(QPair<qreal, qreal>(qPointF(event->localPos()).x(), qPointF(event->localPos()).y()));
			}
		}
	}
	PaintWidget::mouseMoveEvent(event);
}

void MiscWidget::mousePressEvent(QMouseEvent *event) {

	if (edit_mode == SINGLE_MODE) {

		if (mode == VelocityEditor) {

			// check whether selection has to be changed.
			bool clickHandlesSelected = false;
			foreach (MidiEvent *event, Selection::instance()->selectedEvents()) {

				int velocity = 0;
				NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);

				if (noteOn && noteOn->midiTime() >= matrixWidget->minVisibleMidiTime()
						&& noteOn->midiTime() <= matrixWidget->maxVisibleMidiTime()) {
					velocity = noteOn->velocity();
				}
				if (velocity > 0) {
					int h = (height() * velocity) / 128;
					if (!dragging
							&& mouseInRect(qRectF(event->x() - LEFT_BORDER_MATRIX_WIDGET, height() - h - 5, WIDTH,
										   10))) {
						clickHandlesSelected = true;
						break;
					}
				}
			}

			// find event to select
			bool selectedNew = false;
			if (!clickHandlesSelected) {
				QList<MidiEvent *> *list = matrixWidget->velocityEvents();
				foreach (MidiEvent *event, *list) {

					if (!event->file()->channel(event->channel())->visible()) {
						continue;
					}

					if (event->track()->hidden()) {
						continue;
					}

					int velocity = 0;
					NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
					if (noteOn) {
						velocity = noteOn->velocity();
					}
					if (velocity > 0) {
						qreal h = (height() * velocity) / 128;
						if (!dragging
								&& mouseInRect(qRectF(event->x() - LEFT_BORDER_MATRIX_WIDGET, height() - h - 5, WIDTH,
											   10))) {
							file->protocol()->
							startNewAction("Selection changed",
										   new QImage(":/run_environment/graphics/tool/select_single.png"), false);
							ProtocolEntry *toCopy = _dummyTool->copy();
							EventTool::selectEvent(event, true);
							//matrixWidget->update();
							selectedNew = true;
							_dummyTool->protocol(toCopy, _dummyTool);
							file->protocol()->endAction();
							break;
						}
					}
				}
			}

			// if nothing selected deselect all
			if (Selection::instance()->selectedEvents().size() > 0 && !clickHandlesSelected
					&& !selectedNew) {
				file->protocol()->
				startNewAction("Cleared selection", 0, false);
				ProtocolEntry *toCopy = _dummyTool->copy();
				EventTool::clearSelection();
				_dummyTool->protocol(toCopy, _dummyTool);
				file->protocol()->endAction();
				matrixWidget->update();
			}

			// start drag
			if (Selection::instance()->selectedEvents().size() > 0) {
				dragY = mouseY;
				dragging = true;
			}
		} else {

			//other modes
			trackIndex = -1;
			QList<MidiEvent *> accordingEvents;
			QList<QPair<int, int> > track = getTrack(&accordingEvents);
			for (int i = 0; i < track.size(); i++) {

				qreal xPix = track.at(i).first;
				qreal yPix = height() - (qreal(track.at(i).second) / qreal(_max)) * height();

				if (!dragging && mouseInRect(qRectF(xPix - 4, yPix - 4, 8, 8))) {
					trackIndex = i;

					if (accordingEvents.at(i)) {
						file->protocol()->
						startNewAction("Selection changed",
									   new QImage(":/run_environment/graphics/tool/select_single.png"), false);
						ProtocolEntry *toCopy = _dummyTool->copy();
						EventTool::clearSelection();
						EventTool::selectEvent(accordingEvents.at(i), true, true);
						matrixWidget->update();
						_dummyTool->protocol(toCopy, _dummyTool);
						file->protocol()->endAction();
					}
					break;
				}
			}

			if (trackIndex > -1) {
				dragging = true;
				dragY = mouseY;
			}
		}

	} else if (edit_mode == MOUSE_MODE) {
		freeHandCurve.clear();
		isDrawingFreehand = true;
	} else if (edit_mode == LINE_MODE) {
		lineX = qPointF(event->localPos()).x();
		lineY = qPointF(event->localPos()).y();
		isDrawingLine = true;
	}

	if (event->button() == Qt::LeftButton) {
		PaintWidget::mousePressEvent(event);
		return;
	}
}

void MiscWidget::mouseReleaseEvent(QMouseEvent *event) {

	if (event->button() == Qt::LeftButton) {
		PaintWidget::mouseReleaseEvent(event);
	}

	if (edit_mode == SINGLE_MODE) {
		if (mode == VelocityEditor) {
			if (dragging) {

				dragging = false;

				qreal dX = dragY - mouseY;

				if (dX < -3 || dX > 3) {
					file->protocol()->
					startNewAction("Edited velocity");

					qreal aboveEventVelocity = -1;
					if (aboveEvent) {
						aboveEventVelocity = aboveEvent->velocity();
					}

					qreal dV = 127 * dX / height();
					foreach (MidiEvent *event, Selection::instance()->selectedEvents()) {
						NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
						if (noteOn) {

							qreal v = dV + noteOn->velocity();
							if (aboveEventVelocity > 0) {
								v *= (aboveEventVelocity / noteOn->velocity());
							}

							if (v > 127) {
								v = 127;
							}
							if (v < 0) {
								v = 0;
							}
							noteOn->setVelocity(qFloor(v));
						}
					}

					file->protocol()->endAction();
				}
			}
		} else {
			// other modes
			if (dragging) {

				QList<MidiEvent *> accordingEvents;
				getTrack(&accordingEvents);
				MidiEvent *ev = accordingEvents.at(trackIndex);

				MidiTrack *track = file->track(NewNoteTool::editTrack());
				if (!track) {
					return;
				}

				qreal dX = dragY - mouseY;
				if (dX < -3 || dX > 3) {

					int v = qRound(value(mouseY));

					QString text = "";
					switch (mode) {
						case ControlEditor: {
							text = "Edited Control Change Events";
							break;
						}
						case PitchBendEditor: {
							text = "Edited Pitch Bend Events";
							break;
						}
						case KeyPressureEditor: {
							text = "Edited Key Pressure Events";
							break;
						}
						case ChannelPressureEditor: {
							text = "Edited Channel Pressure Events";
							break;
						}
					}

					file->protocol()->
					startNewAction(text);

					if (ev) {
						if (v < 0) {
							v = 0;
						}
						switch (mode) {
							case ControlEditor: {
								ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent *>(ev);
								if (ctrl) {
									if (v > 127) {
										v = 127;
									}
									ctrl->setValue(v);
								}
								break;
							}
							case PitchBendEditor: {
								PitchBendEvent *event = qobject_cast<PitchBendEvent *>(ev);
								if (event) {
									if (v > 16383) {
										v = 16383;
									}
									event->setValue(v);
								}
								break;
							}
							case KeyPressureEditor: {
								KeyPressureEvent *event = qobject_cast<KeyPressureEvent *>(ev);
								if (event) {
									if (v > 127) {
										v = 127;
									}
									event->setValue(v);
								}
								break;
							}
							case ChannelPressureEditor: {
								ChannelPressureEvent *event = qobject_cast<ChannelPressureEvent *>(ev);
								if (event) {
									if (v > 127) {
										v = 127;
									}
									event->setValue(v);
								}
								break;
							}
						}

					} else {

						MidiTrack *track = file->track(NewNoteTool::editTrack());
						if (!track) {
							return;
						}

						int tick = matrixWidget->minVisibleMidiTime();

						if (v < 0) {
							v = 0;
						}
						if (tick < 0) {
							tick = 0;
						}
						switch (mode) {
							case ControlEditor: {
								if (v > 127) {
									v = 127;
								}
								ControlChangeEvent *ctrl = new ControlChangeEvent(channel, controller, v,
										track);
								file->insertEventInChannel(channel, ctrl, tick);
								break;
							}
							case PitchBendEditor: {
								if (v > 16383) {
									v = 16383;
								}
								PitchBendEvent *event = new PitchBendEvent(channel, v, track);
								file->insertEventInChannel(channel, event, tick);
								break;
							}
							case KeyPressureEditor: {
								if (v > 127) {
									v = 127;
								}
								KeyPressureEvent *event = new KeyPressureEvent(channel, v, controller, track);
								file->insertEventInChannel(channel, event, tick);
								break;
							}
							case ChannelPressureEditor: {
								if (v > 127) {
									v = 127;
								}
								ChannelPressureEvent *event = new ChannelPressureEvent(channel, v, track);
								file->insertEventInChannel(channel, event, tick);
								break;
							}
						}
					}

					file->protocol()->endAction();
				}

				dragging = false;
				trackIndex = -1;
			} else {

				// insert new event
				int tick = file->tick(matrixWidget->msOfXPos(
							   mouseX + LEFT_BORDER_MATRIX_WIDGET));
				int v = qRound(value(mouseY));

				QString text = "";
				switch (mode) {
					case ControlEditor: {
						text = "Inserted Control Change Event";
						break;
					}
					case PitchBendEditor: {
						text = "Inserted Pitch Bend Event";
						break;
					}
					case KeyPressureEditor: {
						text = "Inserted Key Pressure Event";
						break;
					}
					case ChannelPressureEditor: {
						text = "Inserted Channel Pressure Event";
						break;
					}
				}

				file->protocol()->startNewAction(text);

				MidiTrack *track = file->track(NewNoteTool::editTrack());
				if (!track) {
					return;
				}
				if (v < 0) {
					v = 0;
				}
				if (tick < 0) {
					tick = 0;
				}
				switch (mode) {
					case ControlEditor: {
						if (v > 127) {
							v = 127;
						}
						ControlChangeEvent *ctrl = new ControlChangeEvent(channel, controller, v,
								track);
						file->insertEventInChannel(channel, ctrl, tick);
						break;
					}
					case PitchBendEditor: {
						if (v > 16383) {
							v = 16383;
						}
						PitchBendEvent *event = new PitchBendEvent(channel, v, track);
						file->insertEventInChannel(channel, event, tick);
						break;
					}
					case KeyPressureEditor: {
						if (v > 127) {
							v = 127;
						}
						KeyPressureEvent *event = new KeyPressureEvent(channel, v, controller, track);
						file->insertEventInChannel(channel, event, tick);
						break;
					}
					case ChannelPressureEditor: {
						if (v > 127) {
							v = 127;
						}
						ChannelPressureEvent *event = new ChannelPressureEvent(channel, v, track);
						file->insertEventInChannel(channel, event, tick);
						break;
					}
				}

				file->protocol()->endAction();
			}
		}
	}

	QList<QPair<qreal, qreal> > toAlign;

	if (edit_mode == MOUSE_MODE || edit_mode == LINE_MODE) {

		// get track data
		if (edit_mode == MOUSE_MODE) {
			if (isDrawingFreehand) {
				isDrawingFreehand = false;
				// process data
				toAlign = freeHandCurve;
				freeHandCurve.clear();
			}
		} else if (edit_mode == LINE_MODE) {
			if (isDrawingLine) {
				// process data
				isDrawingLine = false;
				if (lineX < mouseX) {
					toAlign.append(QPair<qreal, qreal>(lineX, lineY));
					toAlign.append(QPair<qreal, qreal>(mouseX, mouseY));
				} else if (lineX > mouseX) {
					toAlign.append(QPair<qreal, qreal>(mouseX, mouseY));
					toAlign.append(QPair<qreal, qreal>(lineX, lineY));
				}
			}
		}

		if (toAlign.size() > 0) {

			int minTick = file->tick(matrixWidget->msOfXPos(
							  toAlign.first().first + LEFT_BORDER_MATRIX_WIDGET));
			int maxTick = file->tick(matrixWidget->msOfXPos(
							  toAlign.last().first + LEFT_BORDER_MATRIX_WIDGET));

			// process data
			if (mode == VelocityEditor) {

				// when any events are selected, only use those. Else all
				// in the range
				QList<MidiEvent *> events;
				if (Selection::instance()->selectedEvents().size() > 0) {
					foreach (MidiEvent *event, Selection::instance()->selectedEvents()) {
						NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
						if (noteOn) {
							if (noteOn->midiTime() >= minTick && noteOn->midiTime() <= maxTick) {
								events.append(event);
							}
						}
					}
				} else {
					QList<MidiEvent *> *list = matrixWidget->velocityEvents();
					foreach (MidiEvent *event, *list) {

						if (!file->channel(event->channel())->visible()) {
							continue;
						}

						if (event->track()->hidden()) {
							continue;
						}

						NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
						if (noteOn) {
							if (noteOn->midiTime() >= minTick && noteOn->midiTime() <= maxTick) {
								events.append(event);
							}
						}
					}
				}

				if (events.size() > 0) {

					file->protocol()->startNewAction("Changed velocity");

					// process per event
					foreach (MidiEvent *event, events) {

						NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
						if (noteOn) {

							int tick = noteOn->midiTime();
							int x = matrixWidget->xPosOfMs(file->msOfTick(
															   tick)) - LEFT_BORDER_MATRIX_WIDGET;
							qreal y = interpolate(toAlign, x);

							qreal v = 127 * (height() - y) / height();
							if (v > 127) {
								v = 127;
							}

							noteOn->setVelocity(qFloor(v));
						}
					}
					file->protocol()->endAction();
				}
			} else {

				QString text = "";
				switch (mode) {
					case ControlEditor: {
						text = "Edited Control Change Events";
						break;
					}
					case PitchBendEditor: {
						text = "Edited Pitch Bend Events";
						break;
					}
					case KeyPressureEditor: {
						text = "Edited Key Pressure Events";
						break;
					}
					case ChannelPressureEditor: {
						text = "Edited Channel Pressure Events";
						break;
					}
				}

				file->protocol()->startNewAction(text);

				// remove old events
				QList<MidiEvent *> *list = matrixWidget->velocityEvents();
				for (int i = 0; i < list->size(); i++) {
					if (list->at(i) && list->at(i)->channel() == channel) {
						if (list->at(i)->midiTime() >= minTick && list->at(i)->midiTime() <= maxTick
								&& filter(list->at(i))) {
							file->channel(channel)->removeEvent(list->at(i));
						}
					}
				}

				// compute events
				int stepSize = 10;

				int lastValue = -1;
				for (int tick = minTick; tick <= maxTick; tick += stepSize) {

					qreal x = matrixWidget->xPosOfMs(file->msOfTick(
														 tick)) - LEFT_BORDER_MATRIX_WIDGET;
					qreal y = interpolate(toAlign, int(x));
					int v = qRound(value(y));
					if ((lastValue != -1) && (lastValue == v)) {
						continue;
					}
					MidiTrack *track = file->track(NewNoteTool::editTrack());
					if (!track) {
						return;
					}
					lastValue = v;
					switch (mode) {
						case ControlEditor: {
							ControlChangeEvent *ctrl = new ControlChangeEvent(channel, controller, v,
									track);
							file->insertEventInChannel(channel, ctrl, tick);
							break;
						}
						case PitchBendEditor: {
							PitchBendEvent *event = new PitchBendEvent(channel, v, track);
							file->insertEventInChannel(channel, event, tick);
							break;
						}
						case KeyPressureEditor: {
							KeyPressureEvent *event = new KeyPressureEvent(channel, v, controller, track);
							file->insertEventInChannel(channel, event, tick);
							break;
						}
						case ChannelPressureEditor: {
							ChannelPressureEvent *event = new ChannelPressureEvent(channel, v, track);
							file->insertEventInChannel(channel, event, tick);
							break;
						}
					}
				}

				file->protocol()->endAction();
			}
		}
	}

}

qreal MiscWidget::value(double y) {
	qreal v = _max * (height() - y) / height();
	if (v > _max) {
		v = _max;
	}
	return v;
}

qreal MiscWidget::interpolate(QList<QPair<qreal, qreal> > track, qreal x) {

	for (int i = 0; i < track.size(); i++) {

		if (qFuzzyCompare(track.at(i).first, x)) {
			return qreal(track.at(i).second);
		}

		if (track.at(i).first > x) {

			if (i == 0) {
				return qreal(track.at(i).second);
			} else {

				return qreal(track.at(i - 1).second) +
					   qreal(track.at(i).second - track.at(i - 1).second) * qreal(x - track.at(
								   i - 1).first) / qreal(track.at(i).first - track.at(i - 1).first);
			}
		}
	}

	return 0;
}

void MiscWidget::leaveEvent(QEvent *event) {
	resetState();
	PaintWidget::leaveEvent(event);
}

void MiscWidget::resetState() {

	dragY = -1;
	dragging = false;
	freeHandCurve.clear();
	isDrawingFreehand = false;
	isDrawingLine = false;

	trackIndex = -1;
	update();
}

void MiscWidget::keyPressEvent(QKeyEvent *event) {
	if (Tool::currentTool()) {
		if (Tool::currentTool()->pressKey(event->key())) {
			update();
		}
	}
}

void MiscWidget::keyReleaseEvent(QKeyEvent *event) {
	if (Tool::currentTool()) {
		if (Tool::currentTool()->releaseKey(event->key())) {
			update();
		}
	}
}

QList<QPair<int, int> > MiscWidget::getTrack(QList<MidiEvent *>
		*accordingEvents) {

	QList<QPair<int, int> > track;

	// get list of all events in window
	QList<MidiEvent *> *list = matrixWidget->velocityEvents();

	// get all events before the start tick to find out value before start
	int startTick = matrixWidget->minVisibleMidiTime();
	QMultiMap<int, MidiEvent *> *channelEvents = file->channel(
				channel)->eventMap();
	QMultiMap<int, MidiEvent *>::iterator it = channelEvents->upperBound(startTick);

	bool ok = false;
	int valueBefore = _default;
	MidiEvent *evBef = 0;

	if (channelEvents->size() > 0) {
		bool atEnd = false;
		while (!atEnd) {
			if (it != channelEvents->end() && it.key() <= startTick) {
				QPair<int, int> p = processEvent(it.value(), &ok);
				if (ok) {
					valueBefore = p.second;
					evBef = it.value();
					atEnd = true;
				}
			}
			if (it == channelEvents->begin()) {
				atEnd = true;
			} else {
				it--;
			}
		}
	}
	track.append(QPair<int, int>(0, valueBefore));
	if (accordingEvents) {
		accordingEvents->append(evBef);
	}
	// filter and extract values
	for (int i = list->size() - 1; i >= 0; i--) {
		if (list->at(i) && list->at(i)->channel() == channel) {
			QPair<int, int> p = processEvent(list->at(i), &ok);
			if (ok) {
				if (list->at(i)->midiTime() == startTick) {
					// remove added event
					track.removeFirst();
					if (accordingEvents) {
						accordingEvents->removeFirst();
					}
				}
				track.append(p);
				if (accordingEvents) {
					accordingEvents->append(list->at(i));
				}
			}
		}
	}

	return track;
}

bool MiscWidget::filter(MidiEvent *e) {
	switch (mode) {
		case ControlEditor: {
			ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent *>(e);
			if (ctrl && ctrl->control() == controller) {
				return true;
			} else {
				return false;
			}
		}
		case PitchBendEditor: {
			PitchBendEvent *pitch = qobject_cast<PitchBendEvent *>(e);
			if (pitch) {
				return true;
			} else {
				return false;
			}
		}
		case KeyPressureEditor: {
			KeyPressureEvent *pressure = qobject_cast<KeyPressureEvent *>(e);
			if (pressure && pressure->note() == controller) {
				return true;
			} else {
				return false;
			}
		}
		case ChannelPressureEditor: {
			ChannelPressureEvent *pressure = qobject_cast<ChannelPressureEvent *>(e);
			if (pressure) {
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}

QPair<int, int> MiscWidget::processEvent(MidiEvent *e, bool *isOk) {

	*isOk = false;
	QPair<int, int> pair(-1, -1);
	switch (mode) {
		case ControlEditor: {
			ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent *>(e);
			if (ctrl && ctrl->control() == controller) {
				int x = ctrl->x() - LEFT_BORDER_MATRIX_WIDGET;
				int y = ctrl->value();
				pair.first = x;
				pair.second = y;
				*isOk = true;
			}
			break;
		}
		case PitchBendEditor: {
			PitchBendEvent *pitch = qobject_cast<PitchBendEvent *>(e);
			if (pitch) {
				int x = pitch->x() - LEFT_BORDER_MATRIX_WIDGET;
				int y = pitch->value();
				pair.first = x;
				pair.second = y;
				*isOk = true;
			}
			break;
		}
		case KeyPressureEditor: {
			KeyPressureEvent *pressure = qobject_cast<KeyPressureEvent *>(e);
			if (pressure && pressure->note() == controller) {
				int x = pressure->x() - LEFT_BORDER_MATRIX_WIDGET;
				int y = pressure->value();
				pair.first = x;
				pair.second = y;
				*isOk = true;
			}
			break;
		}
		case ChannelPressureEditor: {
			ChannelPressureEvent *pressure = qobject_cast<ChannelPressureEvent *>(e);
			if (pressure) {
				int x = pressure->x() - LEFT_BORDER_MATRIX_WIDGET;
				int y = pressure->value();
				pair.first = x;
				pair.second = y;
				*isOk = true;
			}
			break;
		}
	}
	return pair;
}

void MiscWidget::computeMinMax() {
	switch (mode) {
		case ControlEditor: {
			_max = 127;
			_default = 0;
			break;
		}
		case PitchBendEditor: {
			_max = 16383;
			_default = 0x2000;
			break;
		}
		case KeyPressureEditor: {
			_max = 127;
			_default = 0;
			break;
		}
		case ChannelPressureEditor: {
			_max = 127;
			_default = 0;
			break;
		}
	}
}
