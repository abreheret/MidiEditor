#ifndef EVENTTOOL_H_
#define EVENTTOOL_H_

#include "EditorTool.h"

#include <QList>
class MidiEvent;

class EventTool: public EditorTool {

	public:
		EventTool();
		EventTool(EventTool &other);
		static void selectEvent(MidiEvent *event, bool single, bool ignoreStr=false);
		static void deselectEvent(MidiEvent *event);
		static void clearSelection();
		void paintSelectedEvents(QPainter *painter);
		static QList<MidiEvent*> *selectedEventList();
		void changeTick(MidiEvent* event, int shiftX);
		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);

		virtual bool pressKey(int key);
		static bool shiftPressed, strPressed;
		static void copyAction();
		static void pasteAction();

	protected:
		static QList<MidiEvent*> *selectedEvents, *copiedEvents;
		QList<MidiEvent*> *ownSelectedEvents;
		static bool isCutAction;
};

#endif
