#pragma once

#include "actions/VR_BrushSelectAction.h"
#include "actions/VR_BrushPaintAction.h"
#include "actions/VR_PlaceAssetAction.h"

const ImVec2 VR_BUTTON_SIZE = ImVec2(0, 60);
constexpr int row_min_height_vr = 43;

namespace GUIVR{
	bool Button(const char* label, const ImVec2& size){

		auto cursor_start = ImGui::GetCursorPos();

		bool result = ImGui::Button(label, size);

		auto drawlist = ImGui::GetForegroundDrawList();
		auto windowSize = ImGui::GetWindowSize();
		auto windowPos = ImGui::GetWindowPos();
		auto cursor_end = ImGui::GetCursorPos();

		ImVec2 min = { 
			windowPos.x + cursor_start.x, 
			windowPos.y + cursor_start.y,
		};
		ImVec2 max = {
			windowPos.x + cursor_start.x + size.x, 
			windowPos.y + cursor_start.y + size.y,
		};
		drawlist->AddRect(min, max, IM_COL32_BLACK, 5.0f, ImDrawCornerFlags_All, 5.0f);

		return result;
	}

	bool ImageButton(
		const char* str_id,
		ImTextureID user_texture_id, 
		const ImVec2& image_size, 
		const ImVec2& uv0, 
		const ImVec2& uv1, 
		const ImVec4& bg_col, 
		const ImVec4& tint_col
	){

		auto cursor_start = ImGui::GetCursorPos();

		bool result = ImGui::ImageButton(str_id, user_texture_id, image_size, uv0, uv1, bg_col, tint_col);

		auto drawlist = ImGui::GetForegroundDrawList();
		auto windowSize = ImGui::GetWindowSize();
		auto windowPos = ImGui::GetWindowPos();
		auto cursor_end = ImGui::GetCursorPos();

		ImVec2 min = { 
			windowPos.x + cursor_start.x + 2.0f, 
			windowPos.y + cursor_start.y + 2.0f,
		};
		ImVec2 max = {
			windowPos.x + cursor_start.x + image_size.x + 6.0f,
			windowPos.y + cursor_start.y + image_size.y + 4.0f,
		};
		
		auto color = IM_COL32_BLACK;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			color = ImGui::GetColorU32(ImGuiCol_ButtonHovered, 1.0f);
		}
		

		drawlist->AddRect(min, max, color, 5.0f, ImDrawCornerFlags_All, 5.0f);

		return result;
	}
}



//void makeVRPanelSwitcher(SplatEditor* editor)
//{
//	const float spacing = ImGui::GetStyle().ItemSpacing.x;
//	const float availableWidth = ImGui::GetContentRegionAvail().x;
//
//	const float firstRowButtonWidth = (availableWidth - 2.0f * spacing) / 3.0f;
//	const float secondRowButtonWidth = (availableWidth - spacing) / 2.0f;
//
//	const ImVec2 firstRowButtonSize(firstRowButtonWidth, 48.0f);
//	const ImVec2 secondRowButtonSize(secondRowButtonWidth, 48.0f);
//
//	if (ImGui::Button("Paint##VRPanel", firstRowButtonSize)) {
//		editor->requestVRPanelMode(VRPanelMode::PAINTING);
//	}
//
//	ImGui::SameLine();
//
//	if (ImGui::Button("Select##VRPanel", firstRowButtonSize)) {
//		editor->requestVRPanelMode(VRPanelMode::BRUSHES);
//	}
//
//	ImGui::SameLine();
//
//	if (ImGui::Button("Layers##VRPanel", firstRowButtonSize)) {
//		editor->requestVRPanelMode(VRPanelMode::LAYERS);
//	}
//
//	if (ImGui::Button("Assets##VRPanel", secondRowButtonSize)) {
//		editor->requestVRPanelMode(VRPanelMode::ASSETS);
//	}
//
//	ImGui::SameLine();
//
//	if (ImGui::Button("Teleport##VRPanel", secondRowButtonSize)) {
//		editor->requestVRPanelMode(VRPanelMode::TELEPORT);
//	}
//
//	ImGui::Separator();
//}
void makeVRPanelSwitcher(SplatEditor* editor)
{
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float availableWidth = ImGui::GetContentRegionAvail().x;

	const float firstRowButtonWidth = (availableWidth - 2.0f * spacing) / 3.0f;
	const float secondRowButtonWidth = (availableWidth - spacing) / 2.0f;

	const ImVec2 firstRowButtonSize(firstRowButtonWidth, 48.0f);
	const ImVec2 secondRowButtonSize(secondRowButtonWidth, 48.0f);

	auto switchPanel = [&](VRPanelMode mode) {
		editor->setAction(nullptr);
		editor->requestVRPanelMode(mode);
		};

	if (ImGui::Button("Paint##VRPanel", firstRowButtonSize)) {
		switchPanel(VRPanelMode::PAINTING);
	}

	ImGui::SameLine();

	if (ImGui::Button("Select##VRPanel", firstRowButtonSize)) {
		switchPanel(VRPanelMode::BRUSHES);
	}

	ImGui::SameLine();

	if (ImGui::Button("Layers##VRPanel", firstRowButtonSize)) {
		switchPanel(VRPanelMode::LAYERS);
	}

	if (ImGui::Button("Assets##VRPanel", secondRowButtonSize)) {
		switchPanel(VRPanelMode::ASSETS);
	}

	ImGui::SameLine();

	if (ImGui::Button("Teleport##VRPanel", secondRowButtonSize)) {
		switchPanel(VRPanelMode::TELEPORT);
	}

	ImGui::Separator();
}

void make_layeritem_vr(SceneNode* node, int i){

	auto editor = SplatEditor::instance;

	ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height_vr);
	ImGui::TableNextColumn();

	{ // COLUMN SELECTABLE
		ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
		string id = format("  ##vr_{}_{}", node->name, i);
		if (ImGui::Selectable(id.c_str(), node->selected, selectable_flags, VR_BUTTON_SIZE)) {
			editor->scene.deselectAllNodes();
			node->selected = true;

			if (ImGui::GetIO().KeyCtrl){
				// TODO: on ctrl+click, all primitives of the node should be selected
				// setSelected(gd);
			}

			// updateBoundingBox(node);
		}

		if (node->selected) {
			ImGui::SetItemDefaultFocus();
		}

		ImGui::SameLine();
		float cy = ImGui::GetCursorPosY();
		//ImGui::SetCursorPosY(cy + 10.0f);
		ImGui::Text(node->name.c_str());
		//ImGui::SetCursorPosY(cy);
	}

	{ // draw selectable outline
		auto drawlist = ImGui::GetForegroundDrawList();
		auto windowSize = ImGui::GetWindowSize();
		auto windowPos = ImGui::GetWindowPos();
		auto cursor_end = ImGui::GetCursorPos();

		ImVec2 min = { windowPos.x + 10.0f, windowPos.y + cursor_end.y - row_min_height_vr - 24.0f};
		ImVec2 max = { windowPos.x + windowSize.x - 10.0f, windowPos.y + cursor_end.y + 20.0f - 24.0f};
		drawlist->AddRect(min, max, IM_COL32_BLACK, 5.0f, ImDrawCornerFlags_All, 5.0f);
	}

	{ // COLUMN ACTIONS
		ImGui::TableNextColumn();

		ImVec2 buttonSize = ImVec2(32.0f, 32.0f);
		float symbolSize = 32.0f;
		float cy = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(cy + 10.0f);

		ImTextureID my_tex_id = (void*)(intptr_t)Runtime::gltex_symbols_32x32;
		ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		
		ImVec2 uv_eye_open_0   = ImVec2(2.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 0.0f * symbolSize);
		ImVec2 uv_eye_open_1   = ImVec2(3.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 1.0f * symbolSize / Runtime::gltex_symbols_32x32_height);
		ImVec2 uv_eye_closed_0 = ImVec2(3.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 0.0f * symbolSize);
		ImVec2 uv_eye_closed_1 = ImVec2(4.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 1.0f * symbolSize / Runtime::gltex_symbols_32x32_height);

		ImVec2 uv_lock_open_0   = ImVec2(0.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 0.0f * symbolSize);
		ImVec2 uv_lock_open_1   = ImVec2(1.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 1.0f * symbolSize / Runtime::gltex_symbols_32x32_height);
		ImVec2 uv_lock_closed_0 = ImVec2(1.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 0.0f * symbolSize);
		ImVec2 uv_lock_closed_1 = ImVec2(2.0f * symbolSize / Runtime::gltex_symbols_32x32_width, 1.0f * symbolSize / Runtime::gltex_symbols_32x32_height);

		ImVec2 uv_eye_0 = uv_eye_open_0;
		ImVec2 uv_eye_1 = uv_eye_open_1;
		if(!node->visible){
			uv_eye_0 = uv_eye_closed_0;
			uv_eye_1 = uv_eye_closed_1;
		}

		ImVec2 uv_lock_0 = uv_lock_open_0;
		ImVec2 uv_lock_1 = uv_lock_open_1;
		if(node->locked){
			uv_lock_0 = uv_lock_closed_0;
			uv_lock_1 = uv_lock_closed_1;
		}

		string label_eye = format("toggle_visibility##{}", i);
		string label_lock = format("toggle_lock##{}", i);

		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
		// ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 255));
		// ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 255));
		

		float cx = ImGui::GetCursorPosX();

		// SHOW / HIDE
		if(ImGui::ImageButton(label_eye.c_str(), my_tex_id, buttonSize, uv_eye_0, uv_eye_1, bg_col, tint_col)){
			node->visible = !node->visible;
		}
		// if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)){
		// 	ImGui::SetTooltip("Show/Hide this layer.");
		// }


		// LOCK / UNLOCK
		ImGui::SameLine();
		ImGui::SetCursorPosX(cx + buttonSize.x + 5);
		if(ImGui::ImageButton(label_lock.c_str(), my_tex_id, buttonSize, uv_lock_0, uv_lock_1, bg_col, tint_col)){
			node->locked = !node->locked;
		}
		// if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)){
		// 	ImGui::SetTooltip("Lock to prevent interacting with this layer.");
		// }

		ImGui::PopStyleColor(1);
		// ImGui::PopStyleColor();
		// ImGui::PopStyleColor();
	}
}

void makeLayersVR(ImguiPage* page){

	auto editor = SplatEditor::instance;
	auto& settings = editor->settings;

	page->width = 440;
	page->height = 780;

	ImGui::SetCurrentContext(page->context);
	ImGuiIO& io = page->context->IO;
	auto drawlist = ImGui::GetForegroundDrawList();

	auto framebuffer = page->framebuffer;
	framebuffer->setSize(page->width, page->height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// We need to override DisplaySize if we want to draw imgui to a texture.
	io.DisplaySize.x = framebuffer->width;
	io.DisplaySize.y = framebuffer->height;

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	editor->imguiStyleVR();

	ImU32 white = IM_COL32(255, 255, 255, 255);
	ImU32 black = IM_COL32(  0,   0,   0, 255);
	ImU32 color = 0;

	ImGuiWindowFlags window_flags = 0;
	window_flags = window_flags | ImGuiWindowFlags_NoTitleBar;
	//window_flags = window_flags | ImGuiWindowFlags_MenuBar;
	window_flags = window_flags | ImGuiWindowFlags_NoResize;
	window_flags = window_flags | ImGuiWindowFlags_NoMove;
	// window_flags = window_flags | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

	auto size_layers = ImVec2(page->width, page->height);
	auto windowPos = ImVec2(0, 0);
	auto buttonSize = ImVec2(0, 60);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(size_layers);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("VR Layers", nullptr, window_flags);
	ImGui::PushFont(editor->font_vr_title);

	ImGui::Text("Layers");
	// ImGui::NewLine();

	float cy = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(cy + 20.0f);

	
	ImGui::PopFont();
	ImGui::PushFont(editor->font_vr_text);
	makeVRPanelSwitcher(editor);

	static ImGuiTableFlags flags =
		ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
		// | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
		| ImGuiTableFlags_NoBordersInBody
		// | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
		| ImGuiTableFlags_SizingFixedFit 
		// | ImGuiTableFlags_SizingStretchProp
		// | ImGuiTableFlags_SizingStretchSame
		| ImGuiTableFlags_ScrollY
		| ImGuiTableFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 10.0f));

	const float actionSpacing = ImGui::GetStyle().ItemSpacing.y;
	const float actionButtonHeight = 60.0f;
	//const float actionAreaHeight = 2.0f * actionButtonHeight + actionSpacing + 20.0f;
	const float actionAreaHeight = 3.0f * actionButtonHeight + 2.0f * actionSpacing + 20.0f;

	float tableHeight = ImGui::GetContentRegionAvail().y - actionAreaHeight;

	if (tableHeight < 100.0f) {
		tableHeight = 100.0f;
	}

	static int selectedIndex = -1;

	// ImVec2 cell_padding = {20.0f, 10.0f};
	// ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);

	if (ImGui::BeginTable("Models##listOfModels", 2, flags, ImVec2(0.0f, tableHeight))) {

		ImGui::TableSetupColumn("Model", ImGuiTableColumnFlags_WidthStretch, 0.0f);
		// ImGui::TableSetupColumn("Mem",     ImGuiTableColumnFlags_WidthFixed, 50.0f);
		ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.0f);

		int i = 0;

		for (auto& node : editor->scene.world->children) {
			if (!node || node->hidden) continue;

			if (auto splatNode = dynamic_pointer_cast<SNSplats>(node)) {
				make_layeritem_vr(splatNode.get(), i++);
			}
			else if (auto pointNode = dynamic_pointer_cast<SNPoints>(node)) {
				make_layeritem_vr(pointNode.get(), i++);
			}
			else if (auto triangleNode = dynamic_pointer_cast<SNTriangles>(node)) {
				make_layeritem_vr(triangleNode.get(), i++);
			}
		}

		ImGui::EndTable();
	}

	// float cy = ImGui::GetCursorPosY();
	shared_ptr<SceneNode> selectedLayer = nullptr;
	shared_ptr<SceneNode> selectionAfterDelete = nullptr;

	shared_ptr<SNSplats> mergeSource = nullptr;
	shared_ptr<SNSplats> mergeTarget = nullptr;

	vector<shared_ptr<SceneNode>> layerNodes;

	for (auto& node : editor->scene.world->children) {
		if (!node || node->hidden) {
			continue;
		}

		if (dynamic_pointer_cast<SNSplats>(node) ||
			dynamic_pointer_cast<SNPoints>(node) ||
			dynamic_pointer_cast<SNTriangles>(node)) {
			layerNodes.push_back(node);
		}
	}

	int selectedLayerIndex = -1;

	for (int i = 0; i < static_cast<int>(layerNodes.size()); ++i) {
		if (!layerNodes[i]->selected) {
			continue;
		}

		selectedLayer = layerNodes[i];
		selectedLayerIndex = i;

		if (i + 1 < static_cast<int>(layerNodes.size())) {
			mergeSource = dynamic_pointer_cast<SNSplats>(layerNodes[i]);
			mergeTarget = dynamic_pointer_cast<SNSplats>(layerNodes[i + 1]);
		}

		break;
	}
	shared_ptr<SNSplats> assetSource = dynamic_pointer_cast<SNSplats>(selectedLayer);

	if (selectedLayerIndex >= 0) {
		if (selectedLayerIndex + 1 < static_cast<int>(layerNodes.size())) {
			selectionAfterDelete = layerNodes[selectedLayerIndex + 1];
		}
		else if (selectedLayerIndex > 0) {
			selectionAfterDelete = layerNodes[selectedLayerIndex - 1];
		}
	}

	const bool canCreateAsset =
		assetSource != nullptr &&
		assetSource->dmng.data.count > 0;

	const bool canMerge =
		mergeSource != nullptr &&
		mergeTarget != nullptr &&
		!mergeSource->locked &&
		!mergeTarget->locked;

	const bool canDelete =
		selectedLayer != nullptr &&
		!selectedLayer->locked &&
		selectedLayer->ID != editor->teleport_environment_layer_id;

	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float availableWidth = ImGui::GetContentRegionAvail().x;
	const float actionButtonWidth = (availableWidth - spacing) / 2.0f;
	const ImVec2 actionButtonSize(actionButtonWidth, actionButtonHeight);

	const ImVec2 actionAreaStart = ImGui::GetCursorPos();

	/*
	 * Row 1:
	 * Add Layer | Create Asset
	 */
	if (ImGui::Button("Add Layer##VR", actionButtonSize)) {
		editor->scene.deselectAllNodes();

		string name = "New Splat Layer";
		shared_ptr<Splats> splats = make_shared<Splats>();
		shared_ptr<SNSplats> node = make_shared<SNSplats>(name, splats);

		editor->scene.world->children.push_back(node);
		editor->setSelectedNode(node.get());
	}

	ImGui::SameLine();
	
	/*
	 * Create Asset 暂时只作为禁用占位。
	 * 下一步再接入 AssetLibrary。
	 */
	/*ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.45f);
	ImGui::Button("Create Asset##VR", actionButtonSize);
	ImGui::PopStyleVar();*/
	if (!canCreateAsset) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.45f);
	}

	const bool createAssetClicked =
		ImGui::Button("Create Asset##VR", actionButtonSize);

	if (!canCreateAsset) {
		ImGui::PopStyleVar();
	}

	if (createAssetClicked && canCreateAsset) {
		editor->createAssetFromLayer(assetSource);
	}

	/*
	 * Row 2:
	 * Merge Down | Delete Layer
	 */
	if (!canMerge) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.45f);
	}

	const bool mergeClicked = ImGui::Button("Merge Down##VR", actionButtonSize);

	if (!canMerge) {
		ImGui::PopStyleVar();
	}

	ImGui::SameLine();

	if (!canDelete) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.45f);
	}

	const bool deleteClicked = ImGui::Button("Delete Layer##VR", actionButtonSize);

	if (!canDelete) {
		ImGui::PopStyleVar();
	}

	if (mergeClicked && canMerge) {
		editor->merge_undoable(mergeSource, mergeTarget);
	}

	if (deleteClicked && canDelete) {
		editor->scene.deselectAllNodes();
		editor->deleteNode_undoable(selectedLayer);

		if (selectionAfterDelete) {
			editor->setSelectedNode(selectionAfterDelete.get());
		}
	}
	/*
	* Row 3:
	* Import Asset
	*/
	const bool importAssetClicked =
		ImGui::Button("Import Asset##VR", ImVec2(availableWidth, actionButtonHeight));

	if (importAssetClicked) {
		editor->importAssetFromFile("E:/models/plant.ply");
	}
	const ImVec2 actionAreaEnd = ImGui::GetCursorPos();

	ImVec2 actionRectMin = {
		windowPos.x + 10.0f,
		windowPos.y + actionAreaStart.y - 5.0f
	};

	ImVec2 actionRectMax = {
		windowPos.x + size_layers.x - 10.0f,
		windowPos.y + actionAreaEnd.y + 5.0f
	};

	drawlist->AddRect(actionRectMin, actionRectMax, black, 5.0f, ImDrawCornerFlags_All, 5.0f);

	ImGui::PopStyleVar();
	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



void makeAssetsVR(ImguiPage* page){

	auto editor = SplatEditor::instance;
	auto& settings = editor->settings;

	int itemsPerRow = 5;
	page->width = 720;
	page->height = 780;

	ImGui::SetCurrentContext(page->context);
	ImGuiIO& io = page->context->IO;
	auto drawlist = ImGui::GetForegroundDrawList();
	
	auto framebuffer = page->framebuffer;
	framebuffer->setSize(page->width, page->height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// We need to override DisplaySize if we want to draw imgui to a texture.
	io.DisplaySize.x = framebuffer->width;
	io.DisplaySize.y = framebuffer->height;

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	editor->imguiStyleVR();

	ImU32 white = IM_COL32(255, 255, 255, 255);
	ImU32 black = IM_COL32(  0,   0,   0, 255);
	ImU32 color = 0;

	ImGuiWindowFlags window_flags = 0;
	window_flags = window_flags | ImGuiWindowFlags_NoTitleBar;
	//window_flags = window_flags | ImGuiWindowFlags_MenuBar;
	window_flags = window_flags | ImGuiWindowFlags_NoResize;
	window_flags = window_flags | ImGuiWindowFlags_NoMove;
	// window_flags = window_flags | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

	auto size_layers = ImVec2(page->width, page->height);
	auto windowPos = ImVec2(0, 0);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(size_layers);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("VR Prototype Assets", nullptr, window_flags);
	ImGui::PushFont(editor->font_vr_title);

	ImGui::Text("Assets");
	//ImGui::Text(u8"Assets \u2122");

	float cy = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(cy + 20.0f);

	ImGui::PopFont();
	ImGui::PushFont(editor->font_vr_text);
	makeVRPanelSwitcher(editor);

	ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec2 uv0 = ImVec2(0.0f, 1.0f);
	ImVec2 uv1 = ImVec2(1.0f, 0.0f);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();
	// ImVec2 thumbSize = ImVec2(128.0f, 128.0f);
	ImVec2 thumbSize = {
		(vMax.x - vMin.x) / float(itemsPerRow) - 15.0f,
		(vMax.x - vMin.x) / float(itemsPerRow) - 15.0f,
	};

	//int i = 0;
	//for(auto node : AssetLibrary::assets){
	//	ImTextureID handle = (void*)(intptr_t)Runtime::gltex_symbols_32x32;

	//	if(node->thumbnail){
	//		handle = (void*)(intptr_t)node->thumbnail->handle;
	//	}

	//	string id = format("{}##{}", node->name, i);
	//	if(i > 0 && (i % itemsPerRow) != 0){
	//		ImGui::SameLine();
	//	}

	//	if(GUIVR::ImageButton(id.c_str(), handle, thumbSize, uv0, uv1, bg_col, tint_col)){
	//		// editor->state.action = ACTION_PLACING;
	//		// editor->setPlacingItem(node);

	//		// TODO: VR Placing Item action
	//		shared_ptr<VR_PlaceAssetAction> action = make_shared<VR_PlaceAssetAction>();
	//		action->placingItem = node;
	//		editor->setAction(action);
	//	}
	//	
	//	i++;
	//}
	int deletingAssetIndex = -1;
	int i = 0;

	for (auto node : AssetLibrary::assets) {
		ImTextureID handle = (void*)(intptr_t)Runtime::gltex_symbols_32x32;

		if (node->thumbnail) {
			handle = (void*)(intptr_t)node->thumbnail->handle;
		}

		if (i > 0 && (i % itemsPerRow) != 0) {
			ImGui::SameLine();
		}

		ImGui::BeginGroup();

		string id = format("{}##{}", node->name, i);

		if (GUIVR::ImageButton(id.c_str(), handle, thumbSize, uv0, uv1, bg_col, tint_col)) {
			shared_ptr<VR_PlaceAssetAction> action = make_shared<VR_PlaceAssetAction>();
			action->placingItem = node;
			editor->setAction(action);
		}

		string deleteId = format("Delete##AssetVR_{}", i);

		if (GUIVR::Button(deleteId.c_str(), ImVec2(thumbSize.x, 42.0f))) {
			editor->setAction(nullptr);
			deletingAssetIndex = i;
		}

		ImGui::EndGroup();

		i++;
	}

	if (deletingAssetIndex >= 0) {
		AssetLibrary::assets.erase(AssetLibrary::assets.begin() + deletingAssetIndex);
	}

	// ImGui::PopStyleVar();
	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void makeBrushesVR(ImguiPage* page){

	auto editor = SplatEditor::instance;
	auto& settings = editor->settings;

	page->width = 440;
	page->height = 580;

	ImGui::SetCurrentContext(page->context);
	ImGuiIO& io = page->context->IO;
	auto drawlist = ImGui::GetForegroundDrawList();
	
	auto framebuffer = page->framebuffer;
	framebuffer->setSize(page->width, page->height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// We need to override DisplaySize if we want to draw imgui to a texture.
	io.DisplaySize.x = framebuffer->width;
	io.DisplaySize.y = framebuffer->height;

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	editor->imguiStyleVR();

	ImU32 white = IM_COL32(255, 255, 255, 255);
	ImU32 black = IM_COL32(  0,   0,   0, 255);
	ImU32 color = 0;


	ImGuiWindowFlags window_flags = 0;
	window_flags = window_flags | ImGuiWindowFlags_NoTitleBar;
	//window_flags = window_flags | ImGuiWindowFlags_MenuBar;
	window_flags = window_flags | ImGuiWindowFlags_NoResize;
	window_flags = window_flags | ImGuiWindowFlags_NoMove;
	// window_flags = window_flags | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

	auto size_layers = ImVec2(page->width, page->height);
	auto windowPos = ImVec2(0, 0);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(size_layers);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("VR Prototype Select & Delete", nullptr, window_flags);
	ImGui::PushFont(editor->font_vr_title);

	 ImGui::Text("Select & Delete");
	//ImGui::Text(u8"Assets \u2122");

	float cy = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(cy + 20.0f);

	ImGui::PopFont();
	ImGui::PushFont(editor->font_vr_text);
	makeVRPanelSwitcher(editor);

	ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec2 uv0 = ImVec2(0.0f, 1.0f);
	ImVec2 uv1 = ImVec2(1.0f, 0.0f);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	ImVec2 buttonSize = {
		(vMax.x - vMin.x) / 3.0f - 12.0f,
		(vMax.x - vMin.x) / 3.0f - 12.0f,
	};


	if(GUIVR::Button("Select", buttonSize)){
		editor->settings.brush.mode = BRUSHMODE::SELECT;

		shared_ptr<VR_BrushSelectAction> action = make_shared<VR_BrushSelectAction>();
		editor->setAction(action);
	}
	
	ImGui::SameLine(); 
	if(GUIVR::Button("Delete", buttonSize)){
		editor->settings.brush.mode = BRUSHMODE::ERASE;

		shared_ptr<VR_BrushSelectAction> action = make_shared<VR_BrushSelectAction>();
		editor->setAction(action);
	}

	ImGui::SameLine();
	if(GUIVR::Button("Duplicate", buttonSize)){
		// editor->settings.brush.mode = BRUSHMODE::REMOVE_FLAGS;

		auto numSelected = editor->getNumSelectedSplats();
		if(numSelected > 0){
			// Duplicate selected splats
			FilterRules rules;
			rules.selection = FILTER_SELECTION_SELECTED;
			editor->filterToNewLayer_undoable(rules);
			
			editor->setAction(nullptr);
		}
	}

	ImGui::Text(" ");

	if(GUIVR::Button("Invert", buttonSize)){
		editor->invertSelection_undoable();
	}

	ImGui::SameLine(); 
	if(GUIVR::Button("Deselect\nAll", buttonSize)){
		editor->deselectAll_undoable();
	}

	ImGui::SameLine(); 
	if(GUIVR::Button("Delete\nSelected", buttonSize)){
		// editor->deleteSelection();
		editor->deleteSelection_undoable();
	}
	
	// ImGui::SliderFloat("Brush Size", &editor->settings.vr_brushSize, 0.01f, 0.3f);

	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void makePaintingVR(ImguiPage* page){

	auto editor = SplatEditor::instance;
	auto& settings = editor->settings;

	page->width = 440;
	page->height = 580;

	ImGui::SetCurrentContext(page->context);
	ImGuiIO& io = page->context->IO;
	auto drawlist = ImGui::GetForegroundDrawList();
	
	auto framebuffer = page->framebuffer;
	framebuffer->setSize(page->width, page->height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// We need to override DisplaySize if we want to draw imgui to a texture.
	io.DisplaySize.x = framebuffer->width;
	io.DisplaySize.y = framebuffer->height;

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	editor->imguiStyleVR();

	ImU32 white = IM_COL32(255, 255, 255, 255);
	ImU32 black = IM_COL32(  0,   0,   0, 255);
	ImU32 color = 0;


	ImGuiWindowFlags window_flags = 0;
	window_flags = window_flags | ImGuiWindowFlags_NoTitleBar;
	//window_flags = window_flags | ImGuiWindowFlags_MenuBar;
	window_flags = window_flags | ImGuiWindowFlags_NoResize;
	window_flags = window_flags | ImGuiWindowFlags_NoMove;
	// window_flags = window_flags | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

	auto size_layers = ImVec2(page->width, page->height);
	auto windowPos = ImVec2(0, 0);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(size_layers);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("VR Painting", nullptr, window_flags);
	ImGui::PushFont(editor->font_vr_title);

	 ImGui::Text("Painting");
	//ImGui::Text(u8"Assets \u2122");

	float cy = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(cy + 20.0f);

	ImGui::PopFont();
	ImGui::PushFont(editor->font_vr_text);

	makeVRPanelSwitcher(editor);

	ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec2 uv0 = ImVec2(0.0f, 1.0f);
	ImVec2 uv1 = ImVec2(1.0f, 0.0f);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	ImVec2 buttonSize = {
		(vMax.x - vMin.x) / 3.0f - 12.0f,
		(vMax.x - vMin.x) / 3.0f - 12.0f,
	};


	if(GUIVR::Button("Paint", buttonSize)){
		shared_ptr<VR_BrushPaintAction> action = make_shared<VR_BrushPaintAction>();
		editor->setAction(action);
	}

	static ImVec4 pickedColor(1.0f, 0.0f, 0.0f, 1.0f);
	ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
	flags |= ImGuiColorEditFlags_NoSmallPreview;
	flags |= ImGuiColorEditFlags_NoSidePreview;
	flags |= ImGuiColorEditFlags_AlphaBar;
	ImGui::ColorPicker4("##picker", (float*)&pickedColor, flags);

	editor->settings.brush.color.r = pickedColor.x;
	editor->settings.brush.color.g = pickedColor.y;
	editor->settings.brush.color.b = pickedColor.z;
	editor->settings.brush.color.a = pickedColor.w;

	ImGui::Text("Mode: ");
	ImGui::SameLine(); ImGui::RadioButton("Normal##guivr", &editor->settings.brushColorMode, BRUSHCOLORMODE_NORMAL);
	ImGui::SameLine(); ImGui::RadioButton("Hue&Saturation##guivr", &editor->settings.brushColorMode, BRUSHCOLORMODE_HUE_SATURATION);

	ImGui::SliderFloat("Brush Size#makePainting", &editor->settings.vr_brushSize, 0.01f, 0.3f);

	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void makeTeleportVR(ImguiPage* page)
{
	auto editor = SplatEditor::instance;
	using TeleportMode = SplatEditor::TeleportMode;
	using SatorDistanceControl = SplatEditor::SatorDistanceControl;

	page->width = 440;
	page->height = 1380;

	ImGui::SetCurrentContext(page->context);
	ImGuiIO& io = page->context->IO;

	auto framebuffer = page->framebuffer;
	framebuffer->setSize(page->width, page->height);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
	glViewport(0, 0, framebuffer->width, framebuffer->height);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	io.DisplaySize.x = framebuffer->width;
	io.DisplaySize.y = framebuffer->height;

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	editor->imguiStyleVR();

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

	const ImVec2 windowSize(page->width, page->height);
	const ImVec2 windowPos(0.0f, 0.0f);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("VR Teleport", nullptr, window_flags);

	ImGui::PushFont(editor->font_vr_title);
	ImGui::Text("Teleport");

	float cy = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(cy + 20.0f);

	ImGui::PopFont();
	ImGui::PushFont(editor->font_vr_text);

	makeVRPanelSwitcher(editor);
	static bool showTeleportEval = false;

	const float pageButtonWidth = ImGui::GetContentRegionAvail().x;
	const ImVec2 pageButtonSize(pageButtonWidth, 55.0f);

	if (GUIVR::Button(
		showTeleportEval ? "Back to Teleport" : "Evaluation Test",
		pageButtonSize)) {

		showTeleportEval = !showTeleportEval;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// ============================================================
	// NORMAL TELEPORT PAGE
	// ============================================================
	if (!showTeleportEval) {

		ImGui::Text("Teleport Mode");

		const float modeButtonWidth = ImGui::GetContentRegionAvail().x;
		const ImVec2 modeButtonSize(modeButtonWidth, 60.0f);

		auto drawModeButton = [&](const char* label, TeleportMode mode) {
			const bool selected = editor->teleport_mode == mode;

			if (selected) {
				ImGui::PushStyleColor(
					ImGuiCol_Button,
					ImGui::GetStyleColorVec4(ImGuiCol_Header)
				);
			}

			if (GUIVR::Button(label, modeButtonSize)) {
				editor->setTeleportMode(mode);
			}

			if (selected) {
				ImGui::PopStyleColor();
			}
			};

		drawModeButton("Direct Splat", TeleportMode::DIRECT_SPLAT);
		drawModeButton("Adaptive Splat", TeleportMode::ADAPTIVE_SPLAT);
		drawModeButton("Refined Splat", TeleportMode::REFINED_SPLAT);
		drawModeButton("SATOR Ray Cursor", TeleportMode::SATOR_RAY_CURSOR);
		drawModeButton("Indicator Teleport", TeleportMode::INDICATOR_TELEPORT);
		drawModeButton("Place Indicator", TeleportMode::PLACE_INDICATOR);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		/*if (editor->teleport_mode == TeleportMode::DIRECT_SPLAT) {
			ImGui::Text("Fixed Search Radius");
			ImGui::SliderFloat("Radius##FixedTeleport", &editor->teleport_fixed_radius, 0.01f, 0.30f, "%.2f m");

			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float width = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;
			const ImVec2 size(width, 55.0f);

			if (GUIVR::Button("Small 1 cm", size)) editor->teleport_fixed_radius = 0.01f;
			ImGui::SameLine();
			if (GUIVR::Button("Large 15 cm", size)) editor->teleport_fixed_radius = 0.15f;
		}*/
		if (editor->teleport_mode == TeleportMode::DIRECT_SPLAT || editor->teleport_mode == TeleportMode::REFINED_SPLAT) {
			ImGui::Text("Search Radius: %.0f cm", editor->teleport_fixed_radius * 100.0f);

			ImGui::SliderFloat("Radius##FixedTeleport", &editor->teleport_fixed_radius, 0.01f, 0.20f, "%.2f m");

			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float width = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;
			const ImVec2 size(width, 55.0f);

			if (GUIVR::Button("3 cm", size)) editor->teleport_fixed_radius = 0.03f;
			ImGui::SameLine();
			if (GUIVR::Button("5 cm", size)) editor->teleport_fixed_radius = 0.05f;

			if (GUIVR::Button("10 cm", size)) editor->teleport_fixed_radius = 0.10f;
			ImGui::SameLine();
			if (GUIVR::Button("20 cm", size)) editor->teleport_fixed_radius = 0.20f;
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Opacity / Scale Threshold: %.2f", editor->teleport_opacity_to_scale_threshold);

			ImGui::Checkbox("Show Accepted / Rejected", &editor->teleport_show_opacity_scale_threshold);

			ImGui::SliderFloat("Threshold##OpacityScale", &editor->teleport_opacity_to_scale_threshold, 0.0f, 20.0f, "%.2f");

			ImGui::Checkbox("Show Opacity/Scale Map", &editor->teleport_show_opacity_scale_debug);

			if (ImGui::Button("Count Accepted / Rejected")) {
				editor->countOpacityScaleCandidates();
			}
		}
		if (editor->teleport_mode == TeleportMode::SATOR_RAY_CURSOR) {
			ImGui::Text("Distance Control");

			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float width = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;
			const ImVec2 size(width, 55.0f);

			const bool joystickSelected =
				editor->teleport_sator_distance_control ==
				SatorDistanceControl::JOYSTICK;

			if (joystickSelected) {
				ImGui::PushStyleColor(
					ImGuiCol_Button,
					ImGui::GetStyleColorVec4(ImGuiCol_Header)
				);
			}

			if (GUIVR::Button("Joystick", size)) {
				editor->teleport_sator_distance_control =
					SatorDistanceControl::JOYSTICK;

				editor->teleport_sator_active = false;
				editor->teleport_sator_tilt_locked = false;
				editor->teleport_sator_cursor_distance =
					editor->teleport_sator_initial_distance;
				editor->teleport_target_valid = false;
			}

			if (joystickSelected) {
				ImGui::PopStyleColor();
			}

			ImGui::SameLine();

			const bool tiltSelected =
				editor->teleport_sator_distance_control ==
				SatorDistanceControl::TILT;

			if (tiltSelected) {
				ImGui::PushStyleColor(
					ImGuiCol_Button,
					ImGui::GetStyleColorVec4(ImGuiCol_Header)
				);
			}

			if (GUIVR::Button("Tilt", size)) {
				editor->teleport_sator_distance_control =
					SatorDistanceControl::TILT;

				editor->teleport_sator_active = false;
				editor->teleport_sator_tilt_locked = false;
				editor->teleport_sator_cursor_distance =
					editor->teleport_sator_initial_distance;
				editor->teleport_target_valid = false;
			}

			if (tiltSelected) {
				ImGui::PopStyleColor();
			}

			ImGui::Text(
				"Cursor Distance: %.2f m",
				editor->teleport_sator_cursor_distance
			);

			ImGui::Text(
				"Range: %.2f - %.2f m",
				editor->teleport_sator_min_distance,
				editor->teleport_sator_max_distance
			);
		}

		/*if (editor->teleport_mode == TeleportMode::DIRECT_SPLAT || editor->teleport_mode == TeleportMode::REFINED_SPLAT) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Opacity Filter");

			ImGui::Checkbox("Enabled##TeleportOpacity", &editor->teleport_opacity_filter_enabled);

			if (editor->teleport_opacity_filter_enabled) {
				ImGui::Text("Threshold: %.2f", editor->teleport_opacity_threshold);

				ImGui::SliderFloat("Opacity##Teleport", &editor->teleport_opacity_threshold, 0.0f, 0.30f, "%.2f");

				const float spacing = ImGui::GetStyle().ItemSpacing.x;
				const float width = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;
				const ImVec2 size(width, 55.0f);

				if (GUIVR::Button("0.01", size)) editor->teleport_opacity_threshold = 0.01f;
				ImGui::SameLine();
				if (GUIVR::Button("0.05", size)) editor->teleport_opacity_threshold = 0.05f;

				if (GUIVR::Button("0.10", size)) editor->teleport_opacity_threshold = 0.10f;
				ImGui::SameLine();
				if (GUIVR::Button("0.20", size)) editor->teleport_opacity_threshold = 0.20f;
			}
		}*/

		if (editor->teleport_mode == TeleportMode::ADAPTIVE_SPLAT) {
			ImGui::Text("Adaptive Search");
			ImGui::SliderFloat("Min Radius", &editor->teleport_adaptive_min_radius, 0.005f, 0.05f, "%.3f m");
			ImGui::SliderFloat("Expansion Start", &editor->teleport_adaptive_expansion_start_distance, 0.5f, 10.0f, "%.1f m");
			ImGui::SliderFloat("Max Radius", &editor->teleport_adaptive_max_radius, 0.03f, 0.30f, "%.2f m");
			ImGui::Text("Fallback: %s", editor->teleport_adaptive_last_used_fallback ? "YES" : "NO");
			ImGui::Text("Selected Radius: %.1f cm", editor->teleport_adaptive_selected_radius * 100.0f);
		}


		ImGui::Text(
			"Indicators: %llu",
			static_cast<unsigned long long>(
				editor->teleport_indicators.size()
				)
		);

		ImGui::SliderFloat(
			"Pick Radius",
			&editor->teleport_indicator_pick_radius,
			0.02f,
			0.50f,
			"%.2f m"
		);

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		const float actionButtonWidth = (availableWidth - spacing) / 2.0f;
		const ImVec2 actionButtonSize(actionButtonWidth, 60.0f);

		if (GUIVR::Button("Remove Last", actionButtonSize)) {
			if (!editor->teleport_indicators.empty()) {
				editor->teleport_indicators.pop_back();
			}

			editor->hovered_teleport_indicator = -1;
			editor->teleport_target_valid = false;
		}

		ImGui::SameLine();

		if (GUIVR::Button("Clear All", actionButtonSize)) {
			editor->teleport_indicators.clear();
			editor->hovered_teleport_indicator = -1;
			editor->teleport_target_valid = false;
		}
		ImGui::Spacing();

		if (GUIVR::Button("Save Indicators", actionButtonSize)) {
			editor->saveTeleportIndicators("teleport_indicators.csv");
		}

		ImGui::SameLine();

		if (GUIVR::Button("Load Indicators", actionButtonSize)) {
			editor->loadTeleportIndicators("teleport_indicators.csv");
		}
	}
	else {
		ImGui::Text("Teleport Evaluation");
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float availableWidth = ImGui::GetContentRegionAvail().x;

		const ImVec2 fullButtonSize(availableWidth, 60.0f);

		const float thirdButtonWidth =
			(availableWidth - 2.0f * spacing) / 3.0f;

		const ImVec2 thirdButtonSize(
			thirdButtonWidth,
			60.0f
		);

		// --------------------------------------------------------
		// Target
		// --------------------------------------------------------

		if (GUIVR::Button("Set Target", fullButtonSize)) {
			editor->setTeleportEvalTargetFromSelection("garden_vase");
		}

		if (editor->teleport_eval_target.name.empty()) {
			ImGui::Text("Target: NOT SET");
		}
		else {
			ImGui::Text(
				"Target: %s",
				editor->teleport_eval_target.name.c_str()
			);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// --------------------------------------------------------
		// Plane calibration using right controller positions
		// --------------------------------------------------------

		ImGui::Text("Evaluation Plane");

		if (GUIVR::Button("Set A", thirdButtonSize)) {
			editor->setTeleportEvalPlaneViewAFromRightController();
		}

		ImGui::SameLine();

		if (GUIVR::Button("Set B", thirdButtonSize)) {
			editor->setTeleportEvalPlaneViewBFromRightController();
		}

		ImGui::SameLine();

		if (GUIVR::Button("Set C", thirdButtonSize)) {
			editor->setTeleportEvalPlaneViewCFromRightController();
		}

		ImGui::Text(
			"A: %s   B: %s   C: %s",
			editor->teleport_eval_target.hasPlaneViewA ? "SET" : "-",
			editor->teleport_eval_target.hasPlaneViewB ? "SET" : "-",
			editor->teleport_eval_target.hasPlaneViewC ? "SET" : "-"
		);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// --------------------------------------------------------
		// Generate 180 viewpoints
		// --------------------------------------------------------

		ImGui::Text(
			"Views: %d",
			editor->teleport_eval_config.numViews
		);

		if (GUIVR::Button("Generate 360 Views", fullButtonSize)) {
			editor->generateTeleportEvalViewsFromPlane();
		}

		ImGui::Text(
			"Generated: %d",
			static_cast<int>(
				editor->teleport_eval_target.viewOrigins.size()
				)
		);

		ImGui::Spacing();

		// --------------------------------------------------------
		// Radius evaluation
		// --------------------------------------------------------

		if (GUIVR::Button("Run Radius 1-30 cm", fullButtonSize)) {
			editor->runTeleportEvalRadiusSweep(1, 30, 1);
		}

		ImGui::Spacing();

		// --------------------------------------------------------
		// Clear
		// --------------------------------------------------------

		if (GUIVR::Button("Clear Eval Views", fullButtonSize)) {
			editor->clearTeleportEvalViews();

			editor->teleport_eval_target.hasPlaneViewA = false;
			editor->teleport_eval_target.hasPlaneViewB = false;
			editor->teleport_eval_target.hasPlaneViewC = false;
		}
	}
	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
