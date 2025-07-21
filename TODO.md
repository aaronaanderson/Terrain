# TODO

## MPE

* Make MPE/nonMPE functionality work without a toggle
  * Make Synthesiser and Voice base classes ( put in morphlib )
* Add a panic()/resetAudio() function to everything
* Create morphlib
  * This will hold code that can be applicable to projects that are not Terrain.
  * Synthesiser and Voice, for example.
  * morphlib would be a static lib for other users ( users that don't intend to modify morphlib )
* Write wrapper library for value tree
  * Use assertions to always verify requests stay on message thread
* Write a class to store an object accessible on either the message thread or the audio thread