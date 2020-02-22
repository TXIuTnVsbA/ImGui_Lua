#pragma once
struct ImFont;
class c_gui {
public:
	bool init() noexcept;

	struct {
		ImFont* tahoma;
		ImFont* segoeui;
		ImFont* Deng;
	} fonts;

};
extern c_gui g_gui;