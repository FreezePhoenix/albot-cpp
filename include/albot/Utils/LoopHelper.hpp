#ifndef ALBOT_LOOP_HELPER_HPP_
#define ALBOT_LOOP_HELPER_HPP_

// Credits to LunarWatcher aka Zoe for this class.
#include "uvw.hpp"
#include <mutex>

#include <functional>
#include <memory>

class LoopHelper {
	private:
		std::shared_ptr<uvw::Loop> loop;
	public:
		using RawTimerCallback = std::function<void(const uvw::TimerEvent&, uvw::TimerHandle&)>;
		using TimerCallback = std::function<void()>;
		using Millis = std::chrono::milliseconds;

		LoopHelper() : loop(uvw::Loop::create()){ }

		/**
		 * Sets a timeout.
		 *
		 * @param callback   The function to call
		 * @param timeout    The amount of time to wait before the callback is executed.
		 *
		 * @returns          A timer pointer you can access. Note that using it isn't required.
		 */
		std::shared_ptr<uvw::TimerHandle> setRawTimeout(RawTimerCallback callback, int timeout) {
			auto timer = loop->resource<uvw::TimerHandle>();
			timer->on<uvw::TimerEvent>([callback](const uvw::TimerEvent& event, auto& handle) {
				callback(event, handle);
				handle.close();
				handle.stop();
			});
			timer->start(Millis(timeout), Millis(0));
			return timer;
		}

		void setTimeout(TimerCallback callback, int timeout) {
			auto timer = loop->resource<uvw::TimerHandle>();
			timer->on<uvw::TimerEvent>([callback](const uvw::TimerEvent& event, auto& handle) {
				callback();
				handle.close();
				handle.stop();
			});
			timer->start(Millis(timeout), Millis(0));
		}

		/**
		 * Set an interval.
		 *
		 * @param callback    The function to call
		 * @param interval    The rate to call the function at, in milliseconds
		 * @param timeout     The timeout before the first call
		 * @returns           A timer pointer you can access. Note that using it isn't required. The handle is also passed
		 *                    to the callback as the second parameter
		 */
		std::shared_ptr<uvw::TimerHandle> setRawInterval(RawTimerCallback callback, int interval, int timeout = -1) {
			if (timeout < 0) timeout = interval;

			auto timer = loop->resource<uvw::TimerHandle>();
			timer->on<uvw::TimerEvent>(callback);
			timer->start(Millis(timeout), Millis(interval));

			return timer;
		}
		void setInterval(TimerCallback callback, int interval) {
			auto timer = loop->resource<uvw::TimerHandle>();
			timer->on<uvw::TimerEvent>([callback](const uvw::TimerEvent& event, auto& handle) {
				callback();
			});
			timer->start(Millis(interval), Millis(interval));
		}

		/**
		 * Creates a job that runs in a separate thread.
		 *
		 * @param callback   The function to call
		 * @returns          A canceleable WorkReq
		 */
		std::shared_ptr<uvw::WorkReq> createJob(std::function<void()> callback) {
			auto job = loop->resource<uvw::WorkReq>(callback);
			job->queue();
			return job;
		}

		/**
		 * This uses libuv's async feature to connect to the main thread.
		 * This MUST be used to do anything that interacts with the loop, like adding a new timeout or creating a new
		 * interval.
		 *
		 * Failing to use this may lead to various problems and incorrect behavior (not necessarily undefined, but I'm not
		 * sure). For an instance, you might end up with
		 */
		void execRaw(std::function<void(const uvw::AsyncEvent& event, uvw::AsyncHandle& handle)> callback) {
			auto asyncHandle = loop->resource<uvw::AsyncHandle>();
			asyncHandle->on<uvw::AsyncEvent>([callback](const auto& evt, auto& handle) {
				callback(evt, handle);
				handle.close();
			});
			asyncHandle->send();
		}
		void exec(std::function<void()> callback) {
			auto asyncHandle = loop->resource<uvw::AsyncHandle>();
			asyncHandle->on<uvw::AsyncEvent>([callback](const auto& evt, auto& handle) {
				callback();
				handle.close();
			});
			asyncHandle->send();
		}

		void run() {
			loop->run<uvw::Loop::Mode::ONCE>();
		}

		/**
		 * This method should only be used for extensions on uvw not defined by this class.
		 */
		std::shared_ptr<uvw::Loop> getLoop() {
			return loop;
		}
		void update() {
			loop->update();
		}
		std::chrono::duration<uint64_t, std::milli> now() {
			return loop->now();
		}
		
	};

#endif /* ALBOT_LOOP_HELPER_HPP_ */