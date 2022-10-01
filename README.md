
# albot-cpp ![GitHub Workflow Status](https://img.shields.io/github/workflow/status/FreezePhoenix/albot-cpp/ubuntu-build?label=ubuntu-build) ![GitHub Workflow Status](https://img.shields.io/github/workflow/status/FreezePhoenix/albot-cpp/alpine-build?label=alpine-build) ![Lines of code](https://img.shields.io/tokei/lines/github/FreezePhoenix/albot-cpp) ![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/FreezePhoenix/albot-cpp) ![GitHub User's stars](https://img.shields.io/github/stars/FreezePhoenix/albot-cpp)

This is an alternative to a JS bot for Adventure Land. Currently in the very early testing phases, I would love feedback.

## Dependencies

The following dependencies need to be installed to compile albot-cpp:

 - nlohmann-json (dev libraries)
 - RapidJSON (dev libraries)
 - LibPOCO (dev, foundation, net, and netssl libraries) 
 - spdlog (runtime and dev libraries)

All dependencies should be installable using `sudo apt-get install`

## Use

### Make sure you clone the repo with `--recurse-submodules --depth=1` or you will have to manually activate the submodules.

All files should be created at the root of the git directory.

First create a bot.json file. Write the following contents:

```json
{"fetch":true, "characters":[]}
```

Secondly, create a .env file, make the first line be your email for Adventure Land, and the second line your password.

Then, run `cmake .` and then `make .`

Once that is done, run `./albot-cpp`, and note that it created a bot.out.json. Edit the `enabled` property of characters to determine which ones to run. Multiple characters should be able to be used, up two 3 fighters and 1 merchant.

Run `mv bot.out.json bot.json` or copy the contents of bot.out.json to bot.json.

Copy CODE/Default to another folder, like CODE/YourScript. Change the lines in bot.json to match.

Customize CODE/YourScript/src/BotImpl.cpp. Currently, this project is *very* early in development, so you have to do almost all of the things yourself. It may be left this way on purpose.

Finally, edit bot.json's fetch property to false (if it isn't already). Then, run `./albot-cpp`, watch it build your code, and then run it!

## FAQ

Q: I can't find a library that's required to compile. What should I do
A: Check to see if you can find it using their instructions on their github. You may have to manually install it. Alternatively, use our docker script!

Q: How do I use the docker scripts?
A: You have to have docker-compose installed. On linux, you should be able to do `docker-compose build albot-cpp-PLATFORM` and replace `PLATFORM` with either `alpine` or `ubuntu`, depending on which platform you want to run on. To start the bot, do `docker-compose up albot-cpp-PLATFORM`.

Q: How does ALBot-CPP build my code?
A: ALBot-CPP uses CMake to build your code. Both CODE projects and SERVICES projects are built in this fashion.

Q: Where's the documentation?
A: We don't have one yet.

Q: Where are all of the builtin functions?
A: ALBot-CPP is designed to be very lightweight, and we only define the bare minimum to interact with the game. We leave it as an exercise to the user to implement more functionality. However, we may have some pre-written functionality in the future, like pathfinding.

## Thanks for using it!

If you do use this, I would be glad to know, I'm `AriaHarper / FreezePhoenix#4333` on discord, if you have any suggestions or changes that you would like to be made, just let me know!

## Credits

Huge thanks to LunarWatcher for the PlayerSkeleton/Player pattern, SocketWrapper class, MovementMath class, and advice on general C++ styling.
Thanks to AVDG for supplying the original docker scripts.
