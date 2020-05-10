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
			}
			break;

		case types::bios_info:
			{
				auto* const x = reinterpret_cast<bios_info*>(header);

				if (x->length == 0)
					break;
				json_hwid["bios_info"]["vendor"] = strings[x->vendor];
				json_hwid["bios_info"]["version"] = strings[x->version];
			}
			break;


		case types::memory_device:
			{
				auto* const x = reinterpret_cast<mem_device*>(header);

				if (x->total_width == 0)
					break;
				json_hwid["memory_device"]["manufacturer"] = strings[x->manufacturer];
				json_hwid["memory_device"]["serial_number"] = strings[x->serial_number];
				json_hwid["memory_device"]["part_number"] = strings[x->part_number];
			}
			break;
		case types::processor_info:
			{
				auto* const x = reinterpret_cast<proc_info*>(header);

				if (x->length == 0)
					break;
				json_hwid["processor_info"]["manufacturer"] = strings[x->manufacturer];
				json_hwid["processor_info"]["version"] = strings[x->version];
				json_hwid["processor_info"]["id"] = std::to_string(static_cast<long>(x->id));
				json_hwid["processor_info"]["cores"] = std::to_string(static_cast<long>(x->cores));
				json_hwid["processor_info"]["threads"] = std::to_string(static_cast<long>(x->threads));
			}
			break;

		default: ;
		}
	}

	HeapFree(heap_handle, 0, smbios_data);

	std::string hex_hwid;
	picosha2::hash256_hex_string(hardware, hex_hwid);
	std::cout << json_hwid.dump() << std::endl;
	std::cout << hardware << std::endl;
	std::cout << hex_hwid << std::endl;

	getchar();
	return 0;
}
