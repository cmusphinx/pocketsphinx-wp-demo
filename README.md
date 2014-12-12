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
- STEP 1: initialize and load (initialize Model and Dictionary, and at least 1 search)
- STEP 2: set search (activate search by name)
- STEP 3: start processing (start the enigine)
- STEP 4: register incomming audio
- STEP 5: wait for the events....
