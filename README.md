# albot-cpp

This is an alternative to a JS bot for Adventure Land. Currently in the very early testing phases, I would love feedback.

## Dependencies

The following dependencies need to be installed to compile albot-cpp:

 - nlohmann-json (dev libraries)
 - RapidJSON (dev libraries)
 - LibPOCO (dev, foundation, net, and netssl libraries) 
 - spdlog (runtime and dev libraries)

All dependencies should be installable using `sudo apt-get install`

## Use

All files should be created at the root of the git directory.

First create a bot.json file. Write the following contents:

```json
{"fetch":true, "characters":[]}
```

Secondly, create a .env file, make the first line be your email for Adventure Land, and the second line your password.

Then, run `cmake .` and then `make .`

Once that is done, run `./albot-cpp`, and note that it created a bot.out.json. Use C-style comments to customize bot.out.json to limit the characters that will be run. Currently, only one character is supported at a time.

Run `mv bot.out.json bot.json` or copy the contents of bot.out.json to bot.json.

Copy CODE/Default to another folder, like CODE/YourScript. Change the lines in bot.json to match.

Customize CODE/YourScript/src/BotImpl.cpp. Currently, this project is *very* early in development, so you have to do almost all of the things yourself. It may be left this way on purpose.

Finally, edit bot.json's fetch property to false. Then, run `./albot-cpp`, watch it build your code, and run it!

## FAQ

Q: I can't find a library that's required to compile. What should I do?
A: Check to see if you can find it using their instructions on their github. You may have to manually install it. Alternatively, use our docker script!

Q: Where are all of the builtin functions?
A: ALBot-CPP is designed to be very lightweight, and we only define the bare minimum to interact with the game. We leave it as an exercise to the user to implement more functionality. However, we may have some pre-written functionality in the future, like pathfinding.

## Thanks for using it!

If you do use this, I would be glad to know, I'm `Aria / Freeze#4333` on discord, if you have any suggestions or changes that you would like to be made, just let me know!

## Credits

Huge thanks to LunarWatcher for the PlayerSkeleton/Player pattern, SocketWrapper class, MovementMath class, and advice on general C++ styling.