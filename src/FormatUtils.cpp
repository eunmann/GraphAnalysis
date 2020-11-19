#include "FormatUtils.hpp"

namespace FormatUtils {
	std::string format_number(double num) {
		std::string formated_string = "";

		char unit[][2] = { "p","n", "u", "m", "", "K", "M", "G", "T" };

		int unit_index = 4;
		if (num < 1.0) {
			for (; unit_index > 0; unit_index--) {
				if (num <= 1e-3) {
					num *= 1e3;
				}
				else {
					break;
				}
			}
		}
		else {
			for (; unit_index < 8; unit_index++) {
				if (num >= 1e3) {
					num *= 1e-3;
				}
				else {
					break;
				}
			}
		}

		char buff[32];
		snprintf(buff, sizeof(buff), "%.3f %s", num, unit[unit_index]);
		return std::string(buff);
	}
}