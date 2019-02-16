#pragma once

#include "util/inlining_util.hpp"
#include "util/macro_helpers.hpp"
#include <atomic>
#include <chrono>
#include <fmt/core.h>
#include <fmt/format.h>
#include <memory>
#include <type_traits>

/**
 * \ingroup libbve-log
 * \def LIBBVE_LOG_DEBUG
 * \brief Enables \link bve::log::Severity::debug Severity::debug \endlink logging messages and adds debug information to logging output.
 */

/**
 * \ingroup libbve-log
 * \def LIBBVE_LOG_NO_SIMPLE_MACRO_NAMES
 * \brief Disables the \ref LOG macro, forcing use of \ref LIBBVE_LOG_LOG.
 */
#ifdef DOXYGEN
#	define LIBBVE_LOG_DEBUG
#	define LIBBVE_LOG_NO_SIMPLE_MACRO_NAMES
#	undef LIBBVE_LOG_NO_SIMPLE_MACRO_NAMES
#endif

namespace bve::log {

	/**
	 * \brief Sets Severity of message for log filtering.
	 */
	enum class Severity {
		/// \brief For extremely verbose debugging information. Requires \ref LIBBVE_LOG_DEBUG <b>Default: Hidden</b>
		debug,
		/// \brief Unimportant information. <b>Default: Hidden</b>
		info,
		/// \brief Can be used to describe the process that caused a warning. <b>Default: Shown</b>
		note,
		/// \brief Something is probably not doing what it was intended for. <b>Default: Shown</b>
		warning,
		/// \brief There is a glaring problem that can be worked around, but poorly. <b>Default: Shown</b>
		error,
		/// \brief The program cannot recover, the operation must be terminated. <b>Default: Shown</b>
		fatal_error
	};

	namespace detail {
		/**
		 * \brief Helper for the integer type underlying \ref log::Severity
		 */
		using SeverityIntType = std::underlying_type_t<Severity>;

		/**
		 * \brief Holds the current time, accurate down to the millisecond.
		 * \details We don't use any of the builtin time types because they lack support for accuracy down to the millisecond.
		 */
		struct CurrentTime {
			int year;
			int month;
			int day;
			int hour;
			int minute;
			int second;
			int millisecond;
		};

		/**
		 * \brief Gets the current time, accurate to the millisecond
		 * \return Current time
		 */
		CurrentTime get_time();

		/**
		 * \brief Atomic container for the current Severity.
		 */
		class CurrentSeverityContainer {
		  public:
			/**
			 * \brief Initializes the container with the default Severity of \ref Severity::note
			 */
			CurrentSeverityContainer() : current_sev_{Severity::note} {}
			/**
			 * @brief Atomically gets the current Severity
			 * @return Current Severity
			 */
			Severity get() const {
				return current_sev_;
			}
			/**
			 * \brief Atomically sets global Severity
			 * \param new_sev New Severity value.
			 */
			void set(Severity const new_sev) {
				current_sev_ = new_sev;
			}

		  private:
			std::atomic<Severity> current_sev_;
		};
	} // namespace detail

	/**
	 * \brief Global object that marks the current Severity
	 */
	inline detail::CurrentSeverityContainer current_severity;

	/**
	 * \ingroup libbve-log
	 * \brief Sends preformatted text to the log. You should not be using this method directly. Use the \ref LOG macro instead.
	 * \details This will block on the file or object used internally for synchronization.
	 * \param fmt_str Preformatted text.
	 */
	void to_log(std::string const& fmt_str);

	/**
	 * \ingroup libbve-log
	 * \brief Chooses the current allowed Severity level for messages. Any message at or above it is allowed through.
	 * \param s Severity to set.
	 */
	FORCE_INLINE void set_visible_severity(Severity const s) {
		current_severity.set(s);
	}

	/**
	 * \ingroup libbve-log
	 * \brief Sets the log output location to a filename.
	 * \details An ostream will be constructed internally to send to the file.
	 *
	 * \param name Relative filename log will be sent to. File will be open in append mode.
	 */
	void set_output_location(std::string const& name);

	/**
	 * \ingroup libbve-log
	 * \brief Sets the log output location to an ostream.
	 * \details Shared pointer allows you to get the handle back which can be useful for testing and information gathering.
	 * \param stream The ostream object the log file should write to.
	 */
	void set_output_location(std::shared_ptr<std::ostream> stream);

	/**
	 * \ingroup libbve-log
	 * \brief Returns a shared pointer to the ostream object the log is writing to.
	 */
	std::shared_ptr<std::ostream> get_output_stream();

} // namespace bve::log

// ReSharper disable CppInconsistentNaming
#ifdef LIBBVE_LOG_DEBUG
#	define LIBBVE_LOG_FORMAT_CALL(ser, fmt_str, ...)                                                                                      \
		::fmt::format(fmt("{:0>4d}.{:0>2d}.{:0>2d} {:0>2d}:{:0>2d}:{:0>2d}.{:0>3d}: {:s}: "                                                \
		                  "{:s}:{:d}: " fmt_str "\n"),                                                                                     \
		              time.year, time.month, time.day, time.hour, time.minute, time.second, time.millisecond, #ser, __FILE__, __LINE__,    \
		              __VA_ARGS__)
#else
#	define LIBBVE_LOG_FORMAT_CALL(ser, fmt_str, ...)                                                                                      \
		::fmt::format(fmt("{:0>4d}.{:0>2d}.{:0>2d} {:0>2d}:{:0>2d}:{:0>2d}.{:0>3d}: "                                                      \
		                  "{:s}: " fmt_str "\n"),                                                                                          \
		              time.year, time.month, time.day, time.hour, time.minute, time.second, time.millisecond, #ser, __VA_ARGS__)
#endif

#define LIBBVE_LOG_LOG_SEVERITY_IMPL(ser, fmt_str, ...)                                                                                    \
	{                                                                                                                                      \
		auto const time = ::bve::log::detail::get_time();                                                                                  \
		auto const formated_str = EXPAND(LIBBVE_LOG_FORMAT_CALL(ser, fmt_str, __VA_ARGS__));                                               \
		::bve::log::to_log(std::move(formated_str));                                                                                       \
	}
#ifdef LIBBVE_LOG_DEBUG
#	define LIBBVE_LOG_LOG_SEVERITY_debug(...)                                                                                             \
		if (static_cast<::log::detail::severity_int_type>(::log::current_severity.get())                                                   \
		    <= static_cast<::log::detail::severity_int_type>(::log::Severity::debug)) {                                                    \
			EXPAND(LIBBVE_LOG_LOG_SEVERITY_IMPL(debug, __VA_ARGS__))                                                                       \
		}
#else
#	define LIBBVE_LOG_LOG_SEVERITY_debug(...)
#endif
#define LIBBVE_LOG_LOG_SEVERITY_info(...)                                                                                                  \
	if (static_cast<::bve::log::detail::severity_int_type>(::bve::log::current_severity.get())                                             \
	    <= static_cast<::bve::log::detail::severity_int_type>(::bve::log::Severity::info)) {                                               \
		EXPAND(LIBBVE_LOG_LOG_SEVERITY_IMPL(info, __VA_ARGS__))                                                                            \
	}
#define LIBBVE_LOG_LOG_SEVERITY_note(...)                                                                                                  \
	if (static_cast<::bve::log::detail::severity_int_type>(::bve::log::current_severity.get())                                             \
	    <= static_cast<::bve::log::detail::severity_int_type>(::bve::log::Severity::note)) {                                               \
		EXPAND(LIBBVE_LOG_LOG_SEVERITY_IMPL(note, __VA_ARGS__))                                                                            \
	}
#define LIBBVE_LOG_LOG_SEVERITY_warning(...)                                                                                               \
	if (static_cast<::bve::log::detail::severity_int_type>(::bve::log::current_severity.get())                                             \
	    <= static_cast<::bve::log::detail::severity_int_type>(::bve::log::Severity::warning)) {                                            \
		EXPAND(LIBBVE_LOG_LOG_SEVERITY_IMPL(warning, __VA_ARGS__))                                                                         \
	}
#define LIBBVE_LOG_LOG_SEVERITY_error(...)                                                                                                 \
	if (static_cast<::bve::log::detail::severity_int_type>(::bve::log::current_severity.get())                                             \
	    <= static_cast<::bve::log::detail::severity_int_type>(::bve::log::Severity::error)) {                                              \
		EXPAND(LIBBVE_LOG_LOG_SEVERITY_IMPL(error, __VA_ARGS__))                                                                           \
	}
#define LIBBVE_LOG_LOG_SEVERITY_fatal_error(...)                                                                                           \
	if (static_cast<::bve::log::detail::severity_int_type>(::bve::log::current_severity.get())                                             \
	    <= static_cast<::bve::log::detail::severity_int_type>(::bve::log::Severity::fatal_error)) {                                        \
		EXPAND(LIBBVE_LOG_LOG_SEVERITY_IMPL(fatal_error, __VA_ARGS__))                                                                     \
	}
// ReSharper restore CppInconsistentNaming

/**
 * \ingroup libbve-log
 * \brief Formats given string and writes it to the log.
 * \param sev <b>Unqualified</b> member of \ref bve::log::Severity to set for this message.
 * \param format_str Format string that will be passed directly to fmt.
 * \param ... Arguments to be added into the format string.
 */
#define LIBBVE_LOG_LOG(sev, format_str, ...) EXPAND(CONCAT(LIBBVE_LOG_LOG_SEVERITY_, sev)(format_str, __VA_ARGS__))

#ifndef LIBBVE_LOG_NO_SIMPLE_MACRO_NAMES

// ReSharper disable once CppDoxygenUnresolvedReference
/**
 * \ingroup libbve-log
 * \copydoc LIBBVE_LOG_LOG
 */
#	define LOG(sev, format_str, ...) EXPAND(LIBBVE_LOG_LOG(sev, format_str, __VA_ARGS__))
#endif
