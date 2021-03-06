--lua in C:\Users\%user%\Documents\ImGui\Lua
config.set("window_demo_flag",true)
config.set("collapsing_header_flag",true)
config.set("combo_flag",0)
function print_table ( t )  
    local print_r_cache={}
    local function sub_print_r(t,indent)
        if (print_r_cache[tostring(t)]) then
            print(indent.."*"..tostring(t))
        else
            print_r_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        print(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
                        print(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        print(indent.."["..pos..'] => "'..val..'"')
                    else
                        print(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                print(indent..tostring(t))
            end
        end
    end
    if (type(t)=="table") then
        print(tostring(t).." {")
        sub_print_r(t,"  ")
        print("}")
    else
        sub_print_r(t,"  ")
    end
    print()
end

client.set_event_callback("on_load", function()
	--print("lua_callback_on_load")
end)

client.set_event_callback("on_unload", function()
	--print("lua_callback_on_unload")
end)


client.set_event_callback("on_test", function()
	--print("lua_callback_on_test")
end)

client.set_event_callback("on_draw", function()
	--print("lua_callback_on_draw")
	if(config.get("window_demo_flag")) then
		imgui.show_demo_window("window_demo_flag");
	end
	
	imgui.begin_window("MyDemo")
	imgui.set_window_size(600,650)
	if(imgui.collapsing_header("Demo1")) then
		imgui.text("text")
		imgui.text_colored("text_colored",255,255,0,255)
		imgui.text_disabled("text_disabled")
		imgui.text_wrapped("text_wrapped")
		imgui.label_text("lable_text","lable_text_fmt")
		imgui.bullet()
		imgui.bullet_text("bullet_text")
		if(imgui.button("refresh")) then
			client.refresh()
		end
		imgui.button("button2",100,100)
		imgui.small_button("small_button_lable")
		imgui.color_button("id1",255,255,0,255)
		imgui.color_button("id2",255,255,0,255,0)
		imgui.color_button("id3",255,255,0,255,0,100,100)
		imgui.collapsing_header("collapsing_header1")
		imgui.collapsing_header("collapsing_header2","collapsing_header_flag")
		imgui.collapsing_header("collapsing_header3","collapsing_header_flag",0)
		imgui.checkbox("checkbox1","collapsing_header_flag")
		imgui.checkbox("checkbox2","window_demo_flag")
		imgui.radio_button("radio_button",false)
		imgui.radio_button("radio_button",true)
		imgui.combo("combo","combo_flag","Test1\0Test2\0Test3\0")
	end
	if(imgui.collapsing_header("Demo2")) then
		imgui.drag_float("drag_float1","drag_float1")
		imgui.drag_int("drag_int1","drag_int1")
		
	end
	if(imgui.collapsing_header("Demo3")) then
		imgui.input_text("input_text1","input_text1",true)
		imgui.input_text("input_text2","input_text2",true,32)
		imgui.input_text_multiline("input_text_multiline1","input_text_multiline1")
		imgui.input_float("input_float1","input_float1")
		imgui.input_int("input_int1","input_int1")
		imgui.slider_float("slider_float1","slider_float1",1.0,100.0)
		imgui.slider_angle("slider_angle1","slider_angle1")
		imgui.slider_int("slider_int1","slider_int1",1,100)
		imgui.v_slider_float("v_slider_float1","v_slider_float1",30,100,1,100)
		imgui.v_slider_int("v_slider_int1","v_slider_int1",30,100,1,100)
	end
	imgui.end_window()
end)

--client.refresh()