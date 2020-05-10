#include <ctime>
#include <iostream>
#include <string>

#define NOMINMAX
#include <Windows.h>

#include "contrib/okdshin/picosha2.h"
#include "contrib/nlohmann/json.hpp"
#include "smbios.hpp"

using namespace smbios;
using json = nlohmann::json;

std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	return ltrim(rtrim(str, chars), chars);
}

void remove_substrs(std::string& s, std::string& p)
{
	const auto n = p.length();
	for (auto i = s.find(p);
	     i != std::string::npos;
	     i = s.find(p))
		s.erase(i, n);
}

int main()
{
	// Query size of SMBIOS data.
	const DWORD smbios_data_size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);

	// Allocate memory for SMBIOS data
	auto* const heap_handle = GetProcessHeap();
	auto* const smbios_data = static_cast<raw_smbios_data*>(HeapAlloc(heap_handle, 0,
	                                                                  static_cast<size_t>(smbios_data_size)));
	if (!smbios_data)
	{
		return 0;
	}

	// Retrieve the SMBIOS table
	const DWORD bytes_written = GetSystemFirmwareTable('RSMB', 0, smbios_data, smbios_data_size);
	if (bytes_written != smbios_data_size)
	{
		return 0;
	}

	// Process the SMBIOS data and free the memory under an exit label
	parser meta;
	const BYTE* buff = smbios_data->smbios_table_data;
	const auto buff_size = static_cast<size_t>(smbios_data_size);

	meta.feed(buff, buff_size);

	std::string hardware;
	json json_hwid;
	for (auto& header : meta.headers)
	{
		string_array_t strings;
		parser::extract_strings(header, strings);

		switch (header->type)
		{
		case types::baseboard_info:
			{
				auto* const x = reinterpret_cast<baseboard_info*>(header);

				if (x->length == 0)
					break;
				json_hwid["baseboard_info"]["manufacturer_name"] = strings[x->manufacturer_name];
				json_hwid["baseboard_info"]["product_name"] = strings[x->product_name];
				//json_hwid["baseboard_info"]["version"] = strings[x->version];
				//json_hwid["baseboard_info"]["serial_number"] = strings[x->serial_number];
				//json_hwid["baseboard_info"]["product"] = strings[x->product];
				//json_hwid["baseboard_info"]["version1"] = strings[x->version1];
				//json_hwid["baseboard_info"]["serial_number1"] = strings[x->serial_number1];
			}
			break;

		case types::bios_info:
			{
				auto* const x = reinterpret_cast<bios_info*>(header);

				if (x->length == 0)
					break;
				json_hwid["bios_info"]["vendor"] = strings[x->vendor];
				json_hwid["bios_info"]["version"] = strings[x->version];
				//json_hwid["bios_info"]["starting_segment"] = std::to_string(static_cast<word_t>(x->starting_segment));
				json_hwid["bios_info"]["release_date"] = strings[x->release_date];
				//json_hwid["bios_info"]["rom_size"] = std::to_string(static_cast<byte_t>(x->rom_size));
				//json_hwid["bios_info"]["characteristics"] = std::to_string(static_cast<qword_t>(x->characteristics));
				//json_hwid["bios_info"]["ext_char1"] = std::to_string(static_cast<byte_t>(x->ext_char1));
				//json_hwid["bios_info"]["ext_char2"] = std::to_string(static_cast<byte_t>(x->ext_char2));
				//json_hwid["bios_info"]["sb_major"] = std::to_string(static_cast<byte_t>(x->sb_major));
				//json_hwid["bios_info"]["sb_minor"] = std::to_string(static_cast<byte_t>(x->sb_minor));
				//json_hwid["bios_info"]["ec_major"] = std::to_string(static_cast<byte_t>(x->ec_major));
				//json_hwid["bios_info"]["ec_minor"] = std::to_string(static_cast<byte_t>(x->ec_minor));
			}
			break;

		//case types::memory_device:
		//	{
		//		auto* const x = reinterpret_cast<mem_device*>(header);

		//		if (x->total_width == 0)
		//			break;
		//		//TODO: bad code
		//		// [header->handle]
		//		json_hwid["memory_device"][header->handle]["manufacturer"] = strings[x->manufacturer];
		//		json_hwid["memory_device"][header->handle]["serial_number"] = strings[x->serial_number];
		//		auto* const part_number = strings[x->part_number];
		//		std::string part_number_str(part_number);
		//		json_hwid["memory_device"][header->handle]["part_number"] = trim(part_number_str);
		//	}
		//	break;
		case types::processor_info:
			{
				auto* const x = reinterpret_cast<proc_info*>(header);

				if (x->length == 0)
					break;

				//json_hwid["processor_info"]["type"] = strings[x->type];
				/*	json_hwid["processor_info"]["clock"] = std::to_string(static_cast<word_t>(x->clock));
					json_hwid["processor_info"]["speed_max"] = std::to_string(static_cast<word_t>(x->speed_max));
					json_hwid["processor_info"]["speed_cur"] = std::to_string(static_cast<word_t>(x->speed_cur));
					json_hwid["processor_info"]["l1"] = std::to_string(static_cast<word_t>(x->l1));
					json_hwid["processor_info"]["l2"] = std::to_string(static_cast<word_t>(x->l2));
					json_hwid["processor_info"]["l3"] = std::to_string(static_cast<word_t>(x->l3));
					json_hwid["processor_info"]["characteristics"] = std::to_string(static_cast<word_t>(x->characteristics));*/

				json_hwid["processor_info"][header->handle]["socket_designation"] = strings[x->socket_designation];

				json_hwid["processor_info"][header->handle]["manufacturer"] = strings[x->manufacturer];
				json_hwid["processor_info"][header->handle]["version"] = strings[x->version];
				json_hwid["processor_info"][header->handle]["id"] = std::to_string(static_cast<long>(x->id));
				//json_hwid["processor_info"]["voltage"] = std::to_string(static_cast<byte_t>(x->voltage));
				//json_hwid["processor_info"]["status"] = std::to_string(static_cast<byte_t>(x->status));

				json_hwid["processor_info"][header->handle]["cores"] = std::to_string(static_cast<long>(x->cores));
				//json_hwid["processor_info"]["cores_enabled"] = std::to_string(static_cast<long>(x->cores_enabled));
				json_hwid["processor_info"][header->handle]["threads"] = std::to_string(static_cast<long>(x->threads));
			}
			break;

		default: ;
		}
	}

	HeapFree(heap_handle, 0, smbios_data);


	std::string hwid;
	std::string hwid_sha256;
	hwid.append(json_hwid.dump());
	std::string pattern = "null,";

	remove_substrs(hwid, pattern);

	picosha2::hash256_hex_string(hwid, hwid_sha256);
	std::cout << hwid << std::endl;

	std::cout << hwid_sha256 << std::endl;

	getchar();
	return 0;
}
