# TODO

## MPE

* Make MPE/nonMPE functionality work without a toggle
  * Make Synthesiser and Voice base classes ( put in morphlib )
* Add a panic()/resetAudio() function to everything
* Create morphlib
  * This will hold code that can be applicable to projects that are not Terrain.
  * Synthesiser and Voice, for example.
  * morphlib will be modulare
    * ex: morph_core, morph_
* Write wrapper library for value tree
  * Use assertions to always verify requests stay on message thread
* Write RealTimePtr from scratch