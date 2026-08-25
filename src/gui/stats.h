
void SplatEditor::makeStats(){
	auto editor = SplatEditor::instance;
	auto& settings = editor->settings;
	auto& state = editor->state;

	if(settings.showStats){

		ImVec2 kernelWindowSize = {720, 660};
		ImGui::SetNextWindowPos({10, 100}, ImGuiCond_Once);
		ImGui::SetNextWindowSize(kernelWindowSize, ImGuiCond_Once);

		if(ImGui::Begin("Stats")){
			// ============================================================
			// TELEPORT STATISTICS
			// ============================================================
			// ============================================================
// TELEPORT STATISTICS
// ============================================================
			ImGui::Text("Teleport Statistics");

			ImGui::InputText("Participant ID", editor->teleport_participant_id, sizeof(editor->teleport_participant_id));
			ImGui::InputText("Task", editor->teleport_task_name, sizeof(editor->teleport_task_name));
			ImGui::InputText("Condition", editor->teleport_condition_name, sizeof(editor->teleport_condition_name));

			ImGui::Text("Recording: %s", editor->teleport_recording_enabled ? "ON" : "OFF");

			// ============================================================
			// START RECORDING
			// ============================================================
			if (!editor->teleport_recording_enabled) {
				if (ImGui::Button("Start Teleport Recording")) {
					string participant = string(editor->teleport_participant_id);
					string task = string(editor->teleport_task_name);
					string condition = string(editor->teleport_condition_name);

					if (participant.empty() || task.empty() || condition.empty()) {
						println("[Teleport Log] Please enter Participant ID, Task, and Condition before recording.");
					}
					else {
						participant = editor->sanitizeFilename(participant);
						task = editor->sanitizeFilename(task);
						condition = editor->sanitizeFilename(condition);

						editor->teleport_log_filename = format("teleport_log_{}_{}_{}.csv", participant, task, condition);

						editor->teleport_count = 0;
						editor->teleport_total_distance = 0.0f;
						editor->teleport_records.clear();

						editor->teleport_refinement_activation_count = 0;
						editor->teleport_refined_teleport_count = 0;

						editor->teleport_recording_start_time = std::chrono::steady_clock::now();
						editor->teleport_has_last_record_time = false;

						editor->teleport_recording_enabled = true;

						println("[Teleport Log] Recording started. file={}", editor->teleport_log_filename);
					}
				}
			}
			else {
				// ============================================================
				// STOP RECORDING
				// ============================================================
				if (ImGui::Button("Stop Teleport Recording")) {
					const double totalDuration = std::chrono::duration<double>(std::chrono::steady_clock::now() - editor->teleport_recording_start_time).count();

					editor->teleport_recording_enabled = false;

					println("[Teleport Log] Recording stopped. file={} teleports={} totalDistance={} m duration={} s", editor->teleport_log_filename, editor->teleport_count, editor->teleport_total_distance, totalDuration);
				}
			}

			// ============================================================
			// CURRENT RECORDING INFO
			// ============================================================
			ImGui::Text("Current File: %s", editor->teleport_log_filename.empty() ? "Not set" : editor->teleport_log_filename.c_str());
			ImGui::Text("Teleport Count: %u", editor->teleport_count);
			ImGui::Text("Total Teleport Distance: %.2f m", editor->teleport_total_distance);

			// ============================================================
			// LAST TELEPORT
			// ============================================================
			if (!editor->teleport_records.empty()) {
				const TeleportRecord& lastRecord = editor->teleport_records.back();

				const char* modeName = "Unknown";

				if (lastRecord.mode == TeleportMode::DIRECT_SPLAT) modeName = "Direct Splat";
				else if (lastRecord.mode == TeleportMode::REFINED_SPLAT) modeName = "Refined Splat";
				else if (lastRecord.mode == TeleportMode::INDICATOR_TELEPORT) modeName = "Indicator";
				else if (lastRecord.mode == TeleportMode::PLACE_INDICATOR) modeName = "Place Indicator";
				else if (lastRecord.mode == TeleportMode::ADAPTIVE_SPLAT) modeName = "Adaptive Splat";

				ImGui::Separator();
				ImGui::Text("Last Teleport");

				ImGui::Text("Sequence: %u", lastRecord.sequence);
				ImGui::Text("Elapsed Time: %.3f s", lastRecord.elapsedTime);
				ImGui::Text("Interval: %.3f s", lastRecord.intervalTime);
				ImGui::Text("Mode: %s", modeName);

				if (lastRecord.hasGaussianInfo) {
					ImGui::Text("Gaussian Index: %u", lastRecord.gaussianIndex);
					ImGui::Text("Gaussian Local: %.4f, %.4f, %.4f", lastRecord.gaussianLocalPosition.x, lastRecord.gaussianLocalPosition.y, lastRecord.gaussianLocalPosition.z);
					ImGui::Text("Gaussian World: %.4f, %.4f, %.4f", lastRecord.gaussianWorldPosition.x, lastRecord.gaussianWorldPosition.y, lastRecord.gaussianWorldPosition.z);
					ImGui::Text("Gaussian Scale: %.6f, %.6f, %.6f", lastRecord.gaussianScale.x, lastRecord.gaussianScale.y, lastRecord.gaussianScale.z);
					ImGui::Text("Opacity: %.4f", lastRecord.opacity);
				}
				else {
					ImGui::Text("Gaussian Index: N/A");
					ImGui::Text("Gaussian Local: N/A");
					ImGui::Text("Gaussian World: N/A");
					ImGui::Text("Gaussian Scale: N/A");
					ImGui::Text("Opacity: N/A");
				}

				ImGui::Text("Teleport Target: %.4f, %.4f, %.4f", lastRecord.targetPosition.x, lastRecord.targetPosition.y, lastRecord.targetPosition.z);
				ImGui::Text("Distance: %.4f m", lastRecord.distance);
				ImGui::Text("Refinement Used: %s", lastRecord.refinementUsed ? "YES" : "NO");

				if (lastRecord.refinementUsed) {
					ImGui::Text("Refinement Depth: %.4f m", lastRecord.refinementDepth);
					ImGui::Text("Refinement Height: %.4f m", lastRecord.refinementHeight);
				}
			}
			else {
				ImGui::Text("No teleport recorded.");
			}

			// ============================================================
			// RESET
			// ============================================================
			if (!editor->teleport_recording_enabled) {
				if (ImGui::Button("Reset Teleport Statistics")) {
					editor->teleport_count = 0;
					editor->teleport_total_distance = 0.0f;
					editor->teleport_records.clear();

					editor->teleport_refinement_activation_count = 0;
					editor->teleport_refined_teleport_count = 0;

					editor->teleport_log_filename = "";

					println("[Teleport Log] Statistics reset.");
				}
			}

			ImGui::Separator();
			ImGui::Spacing();
			vec4 color = editor->deviceState.hovered_color;
			uint64_t numSplats = 0;
			scene.process<SNSplats>([&numSplats](SNSplats* node){
				numSplats += node->dmng.data.count;
			});
			static string strRecentKeyInput = "";
			if(Runtime::frame_keys.size() > 0){
				strRecentKeyInput = "";
				for(int i = 0; i < Runtime::frame_keys.size(); i++){
					int key = Runtime::frame_keys[i];
					int scancode = Runtime::frame_actions[i];
					int action = Runtime::frame_mods[i];
					const char* keyname = glfwGetKeyName(key, scancode);
					string strKeyname = "";
					if(keyname != nullptr){
						strKeyname = string(keyname);
					}

					strRecentKeyInput += format("key: {}, code: {}, action: {}, keyname: '{}'", key, scancode, action, strKeyname);

					if(i < Runtime::frame_keys.size() - 1){
						strRecentKeyInput += "\n";
					}

				}
				
			}
			
			struct TableEntry{
				string label;
				string value;
			};

			float deg = GLRenderer::camera->fovy;
			float rad = 3.1415 * deg / 180.0;

			auto locale = getSaneLocale();
			vector<TableEntry> entries = {
				{"FPS"                       , format("{:.1f}", GLRenderer::fps)},
				{"splats"                    , format(locale, "{:L}", numSplats)},
				{"visible splats"            , format(locale, "{:L}", Runtime::numVisibleSplats)},
				{"selected splats"           , format(locale,"{:L}", Runtime::numSelectedSplats)},
				{"visible tile fragments"    , format(locale,"{:L}", Runtime::numVisibleFragments)},
				{"tiled fragments"           , format(locale,"{:L}", Runtime::totalTileFragmentCount)},
				{"splats in largest tile"    , format(locale,"{:L}", editor->deviceState.numSplatsInLargestTile)},
				{"rendered triangles"        , format(locale,"{:L}", Runtime::numRenderedTriangles)},
				{"hovered depth"             , format(locale,"{:.3f}", editor->deviceState.hovered_depth)},
				{"hovered color"             , format(locale,"{:3}, {:3}, {:3}", int(color.r), int(color.g), int(color.b))},
				{"Recent Key Input"          , strRecentKeyInput},
				{"Mouse Position"            , format(locale, "{:5} - {:5}", Runtime::mouseEvents.pos_x,Runtime::mouseEvents.pos_y)},
				{"fovy"                      , format(locale, "{:.1f} deg / {:.4f} rad", deg, rad)},
				{"aspect"                    , format(locale, "{:.3f}", GLRenderer::camera->aspect)},
				{"near"                      , format(locale, "{:.3f}", GLRenderer::camera->near)},
				{"framebuffer"               , format(locale, "{} x {}", GLRenderer::width, GLRenderer::height)},
			};

			{ // Camera Position
				auto pos = Runtime::controls->getPosition();
				string str = format("{:.3}, {:.3}, {:.3} \n", pos.x, pos.y, pos.z);
				entries.push_back({"Camera Position", str});
			}

			{ // Camera
				auto pos = Runtime::controls->getPosition();
				auto target = Runtime::controls->target;

				stringstream ss;
				ss<< std::setprecision(2) << std::fixed;
				ss << format("controls->yaw    = {:.3f};\n", Runtime::controls->yaw);
				ss << format("controls->pitch  = {:.3f};\n", Runtime::controls->pitch);
				ss << format("controls->radius = {:.3f};\n", Runtime::controls->radius);
				ss << format("controls->target = {{ {:.3f}, {:.3f}, {:.3f}, }};\n", target.x, target.y, target.z);
				string str = ss.str();

				entries.push_back({"Camera", str});
			}

			{ // world 
				mat4 t = editor->scene.world->transform;
				string str;
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[0].x, t[0].y, t[0].z, t[0].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[1].x, t[1].y, t[1].z, t[1].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[2].x, t[2].y, t[2].z, t[2].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}", t[3].x, t[3].y, t[3].z, t[3].w);
				entries.push_back({"world->transform", str});
			}

			{ // imn_assets 
				mat4 t = editor->imn_assets->transform;
				string str;
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[0].x, t[0].y, t[0].z, t[0].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[1].x, t[1].y, t[1].z, t[1].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[2].x, t[2].y, t[2].z, t[2].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}", t[3].x, t[3].y, t[3].z, t[3].w);
				entries.push_back({"imn_assets->transform", str});
			}

			{ // imn_brushes 
				mat4 t = editor->imn_brushes->transform;
				string str;
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[0].x, t[0].y, t[0].z, t[0].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[1].x, t[1].y, t[1].z, t[1].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[2].x, t[2].y, t[2].z, t[2].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}", t[3].x, t[3].y, t[3].z, t[3].w);
				entries.push_back({"imn_brushes->transform", str});
			}

			{ // imn_layers 
				mat4 t = editor->imn_layers->transform;
				string str;
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[0].x, t[0].y, t[0].z, t[0].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[1].x, t[1].y, t[1].z, t[1].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[2].x, t[2].y, t[2].z, t[2].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}", t[3].x, t[3].y, t[3].z, t[3].w);
				entries.push_back({"imn_layers->transform", str});
			}

			{ // imn_painting 
				mat4 t = editor->imn_painting->transform;
				string str;
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[0].x, t[0].y, t[0].z, t[0].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[1].x, t[1].y, t[1].z, t[1].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f},\n", t[2].x, t[2].y, t[2].z, t[2].w);
				str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}", t[3].x, t[3].y, t[3].z, t[3].w);
				entries.push_back({"imn_painting->transform", str});
			}

			if(ovr->isActive())
			{ // VR Controllers

				auto flip = glm::dmat4(
					1.0, 0.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, -1.0, 0.0, 0.0,
					0.0, 0.0, 0.0, 1.0
				);
				
				auto handleController = [&](string label, Pose pose, vr::VRControllerState_t state){
					
					if(pose.valid){

						entries.push_back({"    ulButtonPressed"   , format("{}", state.ulButtonPressed)});
						entries.push_back({"    ulButtonTouched"   , format("{}", state.ulButtonTouched)});
						entries.push_back({"    rAxis[0]"          , format("{}, {}", state.rAxis[0].x, state.rAxis[0].y)});
						entries.push_back({"    rAxis[1]"          , format("{}, {}", state.rAxis[1].x, state.rAxis[1].y)});
						entries.push_back({"    rAxis[2]"          , format("{}, {}", state.rAxis[2].x, state.rAxis[2].y)});
						entries.push_back({"    rAxis[3]"          , format("{}, {}", state.rAxis[3].x, state.rAxis[3].y)});
						entries.push_back({"    rAxis[4]"          , format("{}, {}", state.rAxis[4].x, state.rAxis[4].y)});

						mat4 t = flip * pose.transform;

						string str;
						str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}\n", t[0].x, t[0].y, t[0].z, t[0].w);
						str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}\n", t[1].x, t[1].y, t[1].z, t[1].w);
						str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}\n", t[2].x, t[2].y, t[2].z, t[2].w);
						str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}  ", t[3].x, t[3].y, t[3].z, t[3].w);

						entries.push_back({label, str});
					}

				};

				Pose poseLeft = ovr->getLeftControllerPose();
				vr::VRControllerState_t stateLeft = ovr->getLeftControllerState();
				handleController("LEFT VR Controller", poseLeft, stateLeft);

				Pose poseRight = ovr->getRightControllerPose();
				vr::VRControllerState_t stateRight = ovr->getRightControllerState();
				handleController("RIGHT VR Controller", poseRight, stateRight);

			}

			for(auto [label, value] : Runtime::debugValues){
				entries.push_back({label, value});
			}

			for(auto [label, value] : Runtime::debugValueList){
				entries.push_back({label, value});
			}

			entries.push_back({"dbg.numAccepted", format("{:L}", deviceState.dbg.numAccepted)});
			entries.push_back({"dbg.numRejected", format("{:L}", deviceState.dbg.numRejected)});
			
			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

			auto makeTable = [&](vector<TableEntry> entries){
				if (ImGui::BeginTable("Stats", 2, flags)){

					// HEADER
					// ImGui::TableSetupColumn("");
					ImGui::TableSetupColumn("Label");
					ImGui::TableSetupColumn("Value");
					ImGui::TableHeadersRow();

					ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

					int i = 0;
					for(TableEntry entry : entries){
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						string id = format("{}##statentry_{}", entry.label, i);
						bool selected = false;
						if (ImGui::Selectable(id.c_str(), &selected, selectable_flags, ImVec2(0, 0))) {
							glfwSetClipboardString(nullptr, entry.value.c_str());
						}

						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(entry.value.c_str());

						i++;
					}
					
					ImGui::EndTable();
				}
			};
			
			ImGui::Text("Stats (Click to Copy)");
			makeTable(entries);

			//=======================================================
			//=======================================================
			//=======================================================
			ImGui::Text(" ");
			ImGui::Text("Selected Layer: ");
			shared_ptr<SceneNode> node = editor->getSelectedNode();

			if(node == nullptr){
				ImGui::Text("No Layer Selected");
			}else{

				vector<TableEntry> entries;

				{ // TRANSFORM
					mat4 t = node->transform;

					string str;
					str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}\n", t[0].x, t[0].y, t[0].z, t[0].w);
					str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}\n", t[1].x, t[1].y, t[1].z, t[1].w);
					str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}\n", t[2].x, t[2].y, t[2].z, t[2].w);
					str += format("{:10.3f}, {:10.3f}, {:10.3f}, {:10.3f}  ", t[3].x, t[3].y, t[3].z, t[3].w);

					entries.push_back({"transform", str});
				}

				{ // AABB
					Box3 aabb = node->aabb;
					vec3 size = aabb.max - aabb.min;

					string str;
					str += format("min:  {:10.3f}, {:10.3f}, {:10.3f}\n", aabb.min.x, aabb.min.y, aabb.min.z);
					str += format("max:  {:10.3f}, {:10.3f}, {:10.3f}\n", aabb.max.x, aabb.max.y, aabb.max.z);
					str += format("size: {:10.3f}, {:10.3f}, {:10.3f}\n", size.x, size.y, size.z);

					entries.push_back({"AABB", str});
				}

				// SNSPLATS
				onTypeMatch<SNSplats>(node, [&](shared_ptr<SNSplats> splats){
					uint32_t count = splats->dmng.data.count;
					string str = format(getSaneLocale(), "{:L}", count);

					entries.push_back({"#splats", str});
				});

				// SNPoints
				onTypeMatch<SNPoints>(node, [&](shared_ptr<SNPoints> splats){
					uint32_t count = splats->manager.data.count;
					string str = format(getSaneLocale(), "{:L}", count);

					entries.push_back({"#points", str});
				});

				makeTable(entries);
				
			}

		}

		ImGui::End();
	
	}

}