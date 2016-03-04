pocketsphinx-wp-demo
====================

Demo to run pocketsphinx on WP8 platform

Import this demo into VS2013 and run on your phone, it should run fine.

See pocketsphinx documentation for additional details on how to modify the demo.

This demo is hosted at 

http://github.com/cmusphinx/pocketsphinx-wp-demo

This demo created by Toine de Boer, Enbyin (NL)


---------------------------------------------------------------------------------------

HOW IT WORKS: (just build and the demo should work fine)

WasapiAudio == Microphone recorder (Wasapi)
- no HOW TO needed

SpeechRecognizer == PocketSphinx implementation
- STEP 1: initialize (pick one)
- STEP 2: load (at least 1 search)
- STEP 3: set search (activate search by name you just loaded)

For Continuous recognition (get result by events)
- STEP 4.X: start processing (start the engine)
- STEP 5.X: register incomming audio (report microphone audio bytes continuously)
- STEP 6.X: wait for the events.... 

For Single utterance recognition (get instant result)
- STEP 4.Y: get result from a single recording (decode bytes from audio file or finished recording from microphone)