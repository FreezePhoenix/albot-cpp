#include "albot/albot-cpp.hpp"
#include "albot/HttpWrapper.hpp"

std::shared_ptr<spdlog::logger> mLogger = spdlog::stdout_color_mt("ALBotC++");
namespace ALBot {
	std::vector<void*> DLHANDLES = std::vector<void*>();
	std::map<std::string, ServiceInfo> SERVICE_HANDLERS = std::map<std::string, ServiceInfo>();
	std::map<std::string, CharacterGameInfo> CHARACTER_HANDLERS = std::map<std::string, CharacterGameInfo>();
	std::vector<std::thread> CHARACTER_THREADS = std::vector<std::thread>();
	std::vector<std::thread> SERVICE_THREADS = std::vector<std::thread>();

	void clean_code() {
		system("rm CODE/*.so");
		system("rm SERVICES/*.so");
	}

	CharacterGameInfo::HANDLER get_character_handler(const std::string& name) {
		return CHARACTER_HANDLERS.at(name).child_handler;
	}

	void ipc_handler(Message message) {
		mLogger->info("RECEIVED {} FROM {}", message.command, message.requester);
		if (message.command == "code_message") {
			mLogger->info("CODE_MESSAGE TARGET: {}", message.target);
			if (CHARACTER_HANDLERS.contains(message.target)) {
				get_character_handler(message.target)(message);
			} else {
				mLogger->warn("TARGET NOT FOUND. USING FALLBACK.");
				get_character_handler(message.requester)(Message{ "code_message_fail", message.requester, message.target, message.arguments });
			}
		}
	}

	void build_service_code(const std::string& name) {
		std::string FOLDER = fmt::format("SERVICES/{}", name);
		std::string build_type = "Release";
		if (HttpWrapper::config->contains("debug")) {
			if (HttpWrapper::config->at("debug").get<bool>()) {
				build_type = "Debug";
			}
		} else {
			mLogger->warn("config file does not contain a debug flag, assuming false");
		}
		std::string CMAKE = fmt::format("cmake -DCMAKE_BUILD_TYPE={} -DSERVICE_NAME_STRING={} -S {}/. -B {}/.", build_type, name, FOLDER, FOLDER);
		std::string MAKE = fmt::format("make --quiet -C {}/. -j8", FOLDER);
		std::string CP = fmt::format("cp {}/lib{}.so SERVICES/{}.so {}", FOLDER, name, name, NULL_PIPE_OUT);
		mLogger->info("Running CMake on: SERVICES/{}", name);
		system(CMAKE.c_str());
		mLogger->info("Finished. Compiling...");
		system(MAKE.c_str());
		mLogger->info("Finished. Copying...");
		system(CP.c_str());
		mLogger->info("Finished.");
	}

	void build_code(const std::string& name, const std::vector<std::string> names, const std::vector<ClassEnum::CLASS>& classes) {
		std::string FOLDER = "CODE/" + name;
		std::string pretty_names = "";
		std::string char_names = "";
		std::string char_name_ints = "";
		for (size_t i = 0; i < names.size(); i++) {
			pretty_names += names[i];
			char_names += names[i];
			char_name_ints += std::to_string(HttpWrapper::NAME_TO_NUMBER[names[i]]);
			if (i < names.size() - 1) {
				pretty_names += ", ";
				char_names += "\\;";
				char_name_ints += "\\;";
				
			}
		}
		std::string char_klasses = "";
		for (size_t i = 0; i < classes.size(); i++) {
			char_klasses += std::to_string(classes[i]);
			if (i < classes.size() - 1) {
				char_klasses += "\\;";
			}
		}
		std::string build_type = "Release";
		if (HttpWrapper::config->contains("debug")) {
			if (HttpWrapper::config->at("debug").get<bool>()) {
				build_type = "Debug";
			}
		} else {
			mLogger->warn("config file does not contain a debug flag, assuming false");
		}
		std::string CMAKE = fmt::format("cmake -DCMAKE_BUILD_TYPE={} -DNAME_DEFINITIONS=\"{}\" -DCHARACTER_NAME_STRING={} -DCHARACTER_NAME={} -DCHARACTER_CLASS={} -S {}/. -B {}/.", build_type, HttpWrapper::NAME_MACROS, char_names, char_name_ints, char_klasses, FOLDER, FOLDER);
		std::string MAKE = fmt::format("make --quiet -C {}/. -j8", FOLDER);
		mLogger->info("Running CMake on: CODE/{} with character names {}", name, pretty_names);
		system(CMAKE.c_str());
		mLogger->info("Finished. Compiling...");
		system(MAKE.c_str());
		mLogger->info("Finished. Copying...");
		for (const std::string& char_name : names) {
			std::string CP = fmt::format("cp {}/lib{}_{}.so CODE/{}.so {}", FOLDER, name, char_name, char_name, NULL_PIPE_OUT);
			system(CP.c_str());
		}
		mLogger->info("Finished.");
	}

	void start_service(int index) {
		HttpWrapper::Service& service = HttpWrapper::services[index];
		
		ServiceInfo& info = SERVICE_HANDLERS[service.name];
		build_service_code(service.name);
		std::string file = fmt::format("SERVICES/{}.so", service.name);
		void* handle = dlopen(file.c_str(), RTLD_LAZY);
		if (!handle) {
			mLogger->error("Cannot open library: {}", dlerror());
		} else {

			typedef void(*init_t)(ServiceInfo&);
			dlerror();
			init_t init = (init_t)dlsym(handle, "init");

			const char* dlsym_error = dlerror();

			if (dlsym_error) {
				mLogger->error("Cannot load symbol 'init': {}", dlsym_error);
				dlclose(handle);
			}

			info.G = &HttpWrapper::data;
			init(info);
			if (!info.has_destructor()) {
				mLogger->warn("Service {} did not register a destructor! This will cause a memory leak when it exits!", service.name);
			}
			if (!info.has_handler()) {
				mLogger->warn("Service {} did not register a handler! This prevents other processes from communicating with it!", service.name);
			}

			DLHANDLES.push_back(handle);
		}
	}

	void start_character(int index) {
		Character& character = HttpWrapper::characters[index];
		CharacterGameInfo& info = CHARACTER_HANDLERS[character.name];
		int server_index = HttpWrapper::find_server(character.server);
		info.server = HttpWrapper::servers.data() + server_index;
		info.character = HttpWrapper::characters.data() + index;
		info.auth = HttpWrapper::auth;
		info.userId = HttpWrapper::userID;
		info.G = &HttpWrapper::data;
		info.parent_handler = ipc_handler;
		std::string file = "CODE/" + HttpWrapper::characters[index].name + ".so";
		void* handle = dlopen(file.c_str(), RTLD_LAZY);
		if (!handle) {
			mLogger->error("Cannot open library: {}", dlerror());
		} else {
			// load the symbol
			typedef std::thread (*init_t)(CharacterGameInfo&);
			// reset errors
			dlerror();
			init_t init = (init_t)dlsym(handle, "init");

			const char* dlsym_error = dlerror();

			if (dlsym_error) {
				fmt::print("Cannot load symbol 'init': {}\n", dlsym_error);
				mLogger->error("Cannot load symbol 'init': {}", dlsym_error);
				mLogger->flush();
				dlclose(handle);
				return;
			}
			DLHANDLES.push_back(handle);
			CHARACTER_THREADS.emplace_back(init(info));

			if (info.destructor == nullptr) {
				mLogger->warn("Character {} did not register a destructor! This will cause a memory leak when it exits!", character.name);
			}
			if (info.child_handler == nullptr) {
				mLogger->warn("Character {} did not register a handler! This prevents other processes from communicating with it!", character.name);
			}
		}
	}

	std::map<std::string, ServiceInfo>* get_service_handlers() {
		return &ALBot::SERVICE_HANDLERS;
	}
	void login() {
		bool servicesOnly = false;
		nlohmann::json config = nlohmann::json::object();
		if (!HttpWrapper::get_config(config)) {
			exit(1);
		}
		if (config["servicesOnly"].get<bool>()) {
			mLogger->info("Running in services only mode! No connection to the internet will be made, game data will not be updated.");
			servicesOnly = true;
		}
		if (!servicesOnly && !HttpWrapper::login()) {
			exit(1);
		}


		if (!servicesOnly && !config["fetch"].is_null() && config["fetch"].get<bool>()) {
			if (!HttpWrapper::get_characters()) {
				exit(1);
			} else {
				mLogger->info("Writing characters to file...");
				nlohmann::json _chars = nlohmann::json::array();

				for (size_t i = 0; i < HttpWrapper::characters.size(); i++) {
					Character& struct_char = HttpWrapper::characters[i];
					nlohmann::json _char;
					_char["name"] = struct_char.name;
					_char["id"] = struct_char.id;
					_char["script"] = "Default";
					_char["server"] = "US II";
					_char["enabled"] = false;
					_char["type"] = ClassEnum::getClassString(struct_char.klass);
					_chars.push_back(_char);
				}

				config["characters"] = _chars;
				config["fetch"] = false;
				config["runners"] = nlohmann::json::array();

				std::ofstream o("bot.out.json");
				o << config.dump(4) << std::endl;
				mLogger->info("Characters written to file!");
				exit(0);
			}
		}

		if (!servicesOnly && !HttpWrapper::process_characters(config["characters"])) {
			exit(1);
		}

		if (!servicesOnly && !HttpWrapper::get_servers()) {

		}
		HttpWrapper::get_game_data();
		clean_code();
		std::vector<size_t> to_run = std::vector<size_t>();
		to_run.reserve(4);
		int merchants_found = 0;
		int fighters_found = 0;
		for (size_t i = 0; i < HttpWrapper::characters.size(); i++) {
			Character& character = HttpWrapper::characters[i];
			if (character.enabled) {
				switch (character.klass) {
					case ClassEnum::MERCHANT:
						if (merchants_found < 1) {
							merchants_found++;
							to_run.push_back(i);
						} else {
							mLogger->warn("Merchant slots are full!");
						}
						break;
					default:
						if (fighters_found < 3) {
							fighters_found++;
							to_run.push_back(i);
						} else {
							mLogger->warn("Fighter slots are full!");
						}
				}
			}
		}
		for (size_t i = 0; i < HttpWrapper::services.size(); i++) {
			HttpWrapper::Service& service = HttpWrapper::services[i];
			if (service.enabled) {
				start_service(i);
			}
		}
		for (size_t i = 0; i < SERVICE_THREADS.size(); i++) {
			SERVICE_THREADS[i].join();
		}
		if (servicesOnly) {
			SERVICE_HANDLERS.clear();
			return;
		}

		std::map<std::string, std::pair<std::vector<std::string>,std::vector<ClassEnum::CLASS>>> script_to_characters;

		for (size_t character_id : to_run) {
			Character& character = HttpWrapper::characters[character_id];
			auto& entry = script_to_characters[character.script];
			entry.first.push_back(character.name);
			entry.second.push_back(character.klass);
		}

		for (const auto& entry : script_to_characters) {
			build_code(entry.first, entry.second.first, entry.second.second);
		}
		
		for (size_t character_id : to_run) {
			start_character(character_id);
		}
		for (size_t i = 0; i < CHARACTER_THREADS.size(); i++) {
			CHARACTER_THREADS[i].join();
		}
		if (merchants_found == 0 && fighters_found == 0) {
			mLogger->warn("No characters started.");
		}
		CHARACTER_HANDLERS.clear();
		SERVICE_HANDLERS.clear();
		for (size_t i = 0; i < DLHANDLES.size(); i++) {
			dlclose(DLHANDLES[i]);
		}
	}
}

int main() {
	ALBot::login();
}
