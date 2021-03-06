#include "misc_cmds.h"
#include "../Addresses.h"
#include "../../../Common/Common.h"
#include "Server.h"

namespace halo { namespace server { namespace misc { 

	struct versionInfo {
		const char* name;
		DWORD versionId;

		versionInfo(const char* name, DWORD versionId)
			: name(name), versionId(versionId) 
		{
		}

		versionInfo() : name(NULL), versionId(0) {}
	};

	static const std::map<std::string, versionInfo> versionList = 
		[]() -> std::map<std::string, versionInfo>
	{
		std::map<std::string, halo::server::misc::versionInfo> versions;
#ifdef PHASOR_PC
		versions["100"] = halo::server::misc::versionInfo("01.00.00.0564", 0x00006);
		versions["101"] = halo::server::misc::versionInfo("01.00.01.0580", 0x8d9a0);
		versions["102"] = halo::server::misc::versionInfo("01.00.02.0581", 0x8dd88);
		versions["103"] = halo::server::misc::versionInfo("01.00.03.0605", 0x93b48);
		versions["104"] = halo::server::misc::versionInfo("01.00.04.0607", 0x94318);
		versions["105"] = halo::server::misc::versionInfo("01.00.05.0610", 0x956a0);
		versions["106"] = halo::server::misc::versionInfo("01.00.06.0612", 0x956a0);
		versions["107"] = halo::server::misc::versionInfo("01.00.07.0613", 0x956a0);
		versions["108"] = halo::server::misc::versionInfo("01.00.08.0616", 0x96640);
		versions["109"] = halo::server::misc::versionInfo("01.00.09.0620", 0x96640);
#else
		versions["100"] = halo::server::misc::versionInfo("01.00.00.0609", 0x94ecf);
		versions["107"] = halo::server::misc::versionInfo("01.00.07.0613", 0x5bc42f);
		versions["108"] = halo::server::misc::versionInfo("01.00.08.0616", 0x5bcfe7);
		versions["109"] = halo::server::misc::versionInfo("01.00.09.0620", 0x96a27);
	
#endif
		return versions;
	}();

	e_command_result sv_version(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		if (args.size() == 0) {
			out << "valid versions: ";
			for (auto itr = versionList.cbegin(); itr != versionList.cend();
				++itr) {
				out << itr->first << ", ";
			}
			out << endl;
			return e_command_result::kProcessed;
		}

		auto itr = versionList.find(args.ReadString());
		if (itr == versionList.end()) {
			out << "Invalid version specified, see sv_version for list." << endl;
			return e_command_result::kProcessed;
		}

		char* halo_version = (char*)ADDR_BROADCASTVERSION;
		//strcpy_s(halo_version, 14, itr->second.name);
		Common::WriteString(ADDR_BROADCASTVERSION, itr->second.name);

		// Write the version to check against
		Common::WriteBytes(PATCH_CURRENTVERSION, (const LPVOID)&itr->second.versionId, 4);

		out << "Broadcasting on (and accepting) version " << halo_version << ". To disable "
			"version checking, execute sv_version_check." << endl;

		return e_command_result::kProcessed;
	}

	e_command_result sv_version_check(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		static bool enabled = true;
		if (!args.size()) {
			out << "sv_version_check: " << (enabled ? "true" : "false") << endl;
			return e_command_result::kProcessed;
		}

		enabled = args.ReadBool();
	
		if (enabled) {
			BYTE jmp[] = {0xEB};
			Common::WriteBytes(PATCH_ANYVERSIONCHECK1, &jmp, sizeof(jmp));
			Common::WriteBytes(PATCH_ANYVERSIONCHECK2, &jmp, sizeof(jmp));
		} else {
			BYTE jge[] = {0x7D}, jle[] = {0x7e};
			Common::WriteBytes(PATCH_ANYVERSIONCHECK1, &jge, sizeof(jge));
			Common::WriteBytes(PATCH_ANYVERSIONCHECK2, &jle, sizeof(jle));
		}

		out << "Version checking has been " << (enabled ? "enabled" : "disabled") << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_hash_check(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		if (!args.size()) {
			out << "Hash checking is currently " << 
				(getInvalidHashState() ? "disabled" : "enabled") << endl;
			return e_command_result::kProcessed;
		}
		bool state = args.ReadBool();
		server::acceptInvalidHashes(!state);

		out << "Hash checking has been " << (state ? "enabled" : "disabled") << endl;
		return e_command_result::kProcessed;
	}

}}}