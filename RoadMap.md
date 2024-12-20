# In Progress

## 1.2.3
* Fix ASCII Keyboard Steal

## 1.3.0
* MPE Support

## 1.4.0
* MIDI Learn

## 1.5.0
* Multiple Oscillators
* Detuning spread control
* Trajectory spacing spread control
* Panning/Spatial spread control

# Completed

# 1.2.2
* Add Tuning Name - complete
* Double-click default pitch-bend to 2 semitones - complete
* Fix Slider->knob resizing issue - complete
* Add MTS-ESP labeling - complete
* Fix Reaper Presets - complete

## 1.2.1
* Expand envelope ranges
* Attach note velocity to amplitude

## 1.2.0
* Microtonality via MTS-ESP

## 1.1.0
* Presets/Preset Menu - complete
* Randomly Generated Preset button - complete
* Change backend to AudioProcessorValueTreeState - complete
* Add CLAP support - complete
* Add pitch bend range controller - complete

## 1.0.3
* Pitch Bend - complete
* Add Zero-Sustain exception (zero sustain currently causes canceled release phases) - complete
* Experiment/implement more effective DC-Offset filter - complete
* Fix bug that causes Reaper interface to break when loading a preset - complete
* Fix crash that appears in auval stress test (likely this is happening due to the oversampling buffer resizing) - complete

## 1.0.2
* Fix Oversampling bug caused by 1.0.1 update

## 1.0.1
* Fix data allocation to accommodate rapidly changing buffer sizes (A la FL Studio)