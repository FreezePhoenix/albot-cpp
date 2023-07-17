#ifndef BOTIMPL_LIGHTLOOP_HPP_
#define BOTIMPL_LIGHTLOOP_HPP_

#include <functional>
#include <chrono>

struct LightLoop {
	const std::function<void(std::function<void()>, int)> wrapped_interval;
	const std::function<void(std::function<void()>, int)> wrapped_timeout;
	const std::function<void(std::function<void()>)> wrapped_exec;
	const std::function<std::chrono::duration<uint64_t, std::milli>()> wrapped_now;
	LightLoop(std::function<void(std::function<void()>, int)> inter, std::function<void(std::function<void()>, int)> time, const std::function<void(std::function<void()>)> exe, std::function<std::chrono::duration<uint64_t, std::milli>()> now) : wrapped_interval(inter), wrapped_timeout(time), wrapped_exec(exe), wrapped_now(now) {

	}
	void setInterval(std::function<void()> handler, int interval) const {
		wrapped_interval(handler, interval);
	}
	void setTimeout(std::function<void()> handler, int timeout) const {
		wrapped_timeout(handler, timeout);
	}
	void exec(std::function<void()> handler) const {
		wrapped_exec(handler);
	}
	std::chrono::duration<uint64_t, std::milli> now() const {
		return wrapped_now();
	}
};

#endif