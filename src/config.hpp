#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include "structs.h"

#include <cstdio>
#include <array>

namespace pugi
{
	class xml_node;
}

enum EOutputStream
{
	SYSLOG = 0,
	ERRLOG = 1,
	IMLOG = 2,
	MSDP_LOG = 3,
	LAST_LOG = MSDP_LOG
};

template <> EOutputStream ITEM_BY_NAME<EOutputStream>(const std::string& name);
template <> const std::string& NAME_BY_ITEM<EOutputStream>(const EOutputStream spell);

class CLogInfo
{
private:
	CLogInfo() {}
	CLogInfo& operator=(const CLogInfo&);

public:
	static constexpr umask_t UMASK_DEFAULT = -1;

	enum EBuffered
	{
		EB_NO = _IONBF,
		EB_LINE = _IOLBF,
		EB_FULL = _IOFBF
	};

	enum EMode
	{
		EM_REWRITE,
		EM_APPEND
	};

	CLogInfo(const char* filename, const char* human_readable_name) :
		m_handle(nullptr),
		m_filename(filename),
		m_title(human_readable_name),
		m_buffered(EB_LINE),
		m_mode(EM_REWRITE),
		m_umask(UMASK_DEFAULT)
	{
	}
	CLogInfo(const CLogInfo& from) :
		m_handle(nullptr),
		m_filename(from.m_filename),
		m_title(from.m_title),
		m_buffered(from.m_buffered),
		m_mode(from.m_mode),
		m_umask(from.m_umask)
	{
	}

	bool open();

	void buffered(const EBuffered _) { m_buffered = _; }
	void handle(FILE* _) { m_handle = _; }
	void filename(const char* _) { m_filename = _; }
	void mode(const EMode _) { m_mode = _; }
	void umask(const int _) { m_umask = _; }

	auto buffered() const { return m_buffered; }
	const std::string& filename() const { return m_filename; }
	const std::string& title() const { return m_title; }
	FILE* handle() const { return m_handle; }
	auto mode() const { return m_mode; }
	auto umask() const { return m_umask; }

private:
	static constexpr size_t BUFFER_SIZE = 1024;

	FILE *m_handle;
	std::string m_filename;
	std::string m_title;
	EBuffered m_buffered;
	EMode m_mode;
	umask_t m_umask;

	char m_buffer[BUFFER_SIZE];
};

template <> CLogInfo::EBuffered ITEM_BY_NAME<CLogInfo::EBuffered>(const std::string& name);
template <> const std::string& NAME_BY_ITEM<CLogInfo::EBuffered>(const CLogInfo::EBuffered mode);

template <> CLogInfo::EMode ITEM_BY_NAME<CLogInfo::EMode>(const std::string& name);
template <> const std::string& NAME_BY_ITEM<CLogInfo::EMode>(const CLogInfo::EMode mode);

class runtime_config
{
private:
	static const char* CONFIGURATION_FILE_NAME;

	using logs_t = std::array<CLogInfo, 1 + LAST_LOG>;

	runtime_config();
	runtime_config(const runtime_config&);
	runtime_config& operator=(const runtime_config&);

	static void load_from_file(const char* filename);
	static void load_stream_config(CLogInfo& log, const pugi::xml_node* node);

	static logs_t m_logs;
	static std::string m_log_stderr;

public:
	static void load(const char* filename = CONFIGURATION_FILE_NAME) { load_from_file(filename); }
	static bool open_log(const EOutputStream stream);
	static const CLogInfo& logs(EOutputStream id) { return m_logs[static_cast<size_t>(id)]; }
	static void handle(const EOutputStream stream, FILE * handle);
	static const std::string& log_stderr() { return m_log_stderr; }
};

#endif // __CONFIG_HPP__