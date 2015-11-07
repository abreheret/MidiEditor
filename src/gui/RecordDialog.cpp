/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RecordDialog.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include "../midi/MidiFile.h"
#include "../MidiEvent/ChannelPressureEvent.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/UnknownEvent.h"
#include "../MidiEvent/TextEvent.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiChannel.h"

RecordDialog::RecordDialog(MidiFile *file, QMultiMap<int, MidiEvent*> data,
		QWidget *parent) : QDialog(parent)
{
	_data = data;
	_file = file;

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	// label
	QLabel *textlabel = new QLabel("There are "+QString::number(data.size())+
			" recorded Events.\n Choose how to add them to your MidiFile.",
			this);
	layout->addWidget(textlabel, 0,0,1,3);

	// track
	QLabel *tracklabel = new QLabel("Add to Track: ", this);
	layout->addWidget(tracklabel, 1, 0, 1, 1);
	_trackBox = new QComboBox(this);
	for(int i = 0; i< _file->numTracks(); i++){
		_trackBox->addItem(QString::number(i));
	}
	layout->addWidget(_trackBox, 1, 1, 1, 2);

	// channel
	QLabel *channellabel = new QLabel("Add to Channel: ", this);
	layout->addWidget(channellabel, 2, 0, 1, 1);
	_channelBox = new QComboBox(this);
	for(int i = 0; i< 16; i++){
		_channelBox->addItem(QString::number(i+1));
	}
	_channelBox->addItem("given channel (selected by input)");
	_channelBox->setCurrentIndex(16);
	layout->addWidget(_channelBox, 2, 1, 1, 2);

	// ignore types
	QLabel *ignorelabel = new QLabel("Ignore types:", this);
	layout->addWidget(ignorelabel, 3, 0, 1, 3);
	_notes = new QCheckBox("Note on / offEvents", this);
	layout->addWidget(_notes, 4, 0, 1, 3);
	_controlChange = new QCheckBox("ControlChanges", this);
	layout->addWidget(_controlChange, 5, 0, 1, 3);
	_progChange = new QCheckBox("ProgramChanges", this);
	layout->addWidget(_progChange, 6, 0, 1, 3);
	_channelPressure = new QCheckBox("ChannelPressure", this);
	layout->addWidget(_channelPressure, 7, 0, 1, 3);
	_keyPressure = new QCheckBox("KeyPressure", this);
	layout->addWidget(_keyPressure, 8, 0, 1, 3);
	_tempoChange = new QCheckBox("TempoChanges", this);
	layout->addWidget(_tempoChange, 9, 0, 1, 3);
	_tempoChange->setChecked(true);
	_timeSig = new QCheckBox("TimeSignature", this);
	_timeSig->setChecked(true);
	layout->addWidget(_timeSig, 10, 0, 1, 3);
	_text = new QCheckBox("TextEvents", this);
	_text->setChecked(true);
	layout->addWidget(_text, 11, 0, 1, 3);
	_unknown = new QCheckBox("Unknown Events", this);
	layout->addWidget(_unknown, 12, 0, 1, 3);
	_unknown->setChecked(true);

	// buttons
	QPushButton *cancel = new QPushButton("Cancel", this);
	layout->addWidget(cancel, 13, 0, 1, 2);
	connect(cancel, SIGNAL(clicked()), this, SLOT(cancel()));

	QPushButton *ok = new QPushButton("Ok", this);
	layout->addWidget(ok, 13, 2, 1, 1);
	connect(ok, SIGNAL(clicked()), this, SLOT(enter()));
}

void RecordDialog::enter(){

	int channel = _channelBox->currentIndex();
	MidiTrack *track = _file->track(_trackBox->currentIndex());
	bool ownChannel = channel == 16;

	if(_data.size()>0){
		_file->protocol()->startNewAction("Added recorded Events");

		// first enlarge the file ( last event + 1000 ms)
		QMultiMap<int, MidiEvent*>::iterator it = _data.end();
		it--;
		int minLength = it.key()+1000;
		if(minLength>_file->maxTime()){
			_file->setMaxLengthMs(minLength);
		}

		it = _data.begin();
		while(it!=_data.end()){

			int currentChannel = it.value()->channel();
			if(!ownChannel){
				currentChannel = channel;
			}

			// check whether to add event or not
			bool ignoreEvent = false;
			MidiEvent *toCheck = it.value();

			OffEvent *off = dynamic_cast<OffEvent*>(toCheck);
			if(off) {
				toCheck = off->onEvent();
			}

			NoteOnEvent *noteEvent = dynamic_cast<NoteOnEvent*>(toCheck);
			if(noteEvent) {
				ignoreEvent &= _notes->isChecked();
			}

			ChannelPressureEvent *channelPressure =
					dynamic_cast<ChannelPressureEvent*>(toCheck);
			if(channelPressure) {
				ignoreEvent &= _channelPressure->isChecked();
			}

			ControlChangeEvent *control =
					dynamic_cast<ControlChangeEvent*>(toCheck);
			if(control) {
				ignoreEvent &= _controlChange->isChecked();
			}

			KeyPressureEvent *key =
					dynamic_cast<KeyPressureEvent*>(toCheck);
			if(key) {
				ignoreEvent &= _keyPressure->isChecked();
			}

			ProgChangeEvent *prog =
					dynamic_cast<ProgChangeEvent*>(toCheck);
			if(prog) {
				ignoreEvent &= _progChange->isChecked();
			}

			TempoChangeEvent *tempo =
					dynamic_cast<TempoChangeEvent*>(toCheck);
			if(tempo) {
				ignoreEvent &= _tempoChange->isChecked();
				currentChannel = it.value()->channel();
			}

			TimeSignatureEvent *time =
					dynamic_cast<TimeSignatureEvent*>(toCheck);
			if(time) {
				ignoreEvent &= _timeSig->isChecked();
				currentChannel = it.value()->channel();
			}

			TextEvent *text =
					dynamic_cast<TextEvent*>(toCheck);
			if(text) {
				ignoreEvent &= _text->isChecked();
				currentChannel = it.value()->channel();
			}

			UnknownEvent *unknown =
					dynamic_cast<UnknownEvent*>(toCheck);
			if(unknown) {
				ignoreEvent &= _unknown->isChecked();
			}

			if(!ignoreEvent){
				MidiEvent *toAdd = it.value();
				toAdd->setFile(_file);
				toAdd->setChannel(currentChannel, false);
				toAdd->setTrack(track, false);
				_file->channel(toAdd->channel())->insertEvent(toAdd,
						_file->tick(it.key()));
			}
			it++;
		}
		_file->protocol()->endAction();
		qWarning("ok");
	}
	hide();
}

void RecordDialog::cancel(){

	QMessageBox msgBox(this);
	msgBox.setWindowTitle("Remove recorded Events?");
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setText("Do you really wish to remove all recorded Events?");
	QPushButton *connectButton = msgBox.addButton(tr("Remove MidiEvents"),
			QMessageBox::ActionRole);
	msgBox.addButton(tr("Abort"), QMessageBox::ActionRole);

	 msgBox.exec();

	 if (msgBox.clickedButton() == connectButton) {
		 // delete events
		 foreach(MidiEvent *event, _data){
			 delete event;
		 }
	     hide();
	 }
}
