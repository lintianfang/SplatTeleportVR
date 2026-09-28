#pragma once

#include "glm/gtc/matrix_access.hpp"

#ifdef NO_OPENVR
	#include "NOpenVRHelper.h"
#else
	#include "OpenVRHelper.h"
#endif
#include "CudaVirtualMemory.h"

#include "Mesh.h"
#include "./scene/SceneNode.h"
#include "./scene/Scene.h"
#include "./scene/SNSplats.h"
#include "./scene/ImguiNode.h"
#include "./scene/SNPoints.h"
#include "./scene/SNTriangles.h"

#include "cuda.h"
#include "cuda_runtime.h"
#include "CudaModularProgram.h"

#include "OrbitControls.h"
#include "DesktopVRControls.h"
#include "./actions/InputAction.h"

#include "common.h"
#include "AssetLibrary.h"

#include "loader/GSPlyLoader.h"
#include "loader/SplatsyFilesLoader.h"
#include "loader/SplatsyLoader.h"
#include "loader/PLYHeightmapLoader.h"
#include "loader/GLBLoader.h"
#include "loader/LASLoader.h"
#include "loader/PLYMeshLoader.h"
#include "writer/GSPlyWriter.h"
#include "writer/SplatsyWriter.h"
#include "writer/SplatsyFilesWriter.h"
// #include "writer/SplatsyPlyWriter.h"
#include "Runtime.h"
#include "tween.h"

#include "common.h"
#include "utils.h"

struct Action;
struct InputAction;

using glm::transpose;
using glm::vec2;
using glm::quat;
using glm::vec3;

enum class VRPanelMode {
	PAINTING,
	BRUSHES,
	LAYERS,
	ASSETS,
	TELEPORT
};

struct TriangleQueueItem{
	TriangleData geometry;
	TriangleMaterial material;
};

struct GuiVrPage{
	string label;
	int width;
	int height;
};

struct _DrawSphereArgs{
	vec3 pos   = {0.0f, 0.0f, 0.0f};
	vec3 scale = {1.0f, 1.0f, 1.0f};
	vec4 color = {0.0f, 1.0f, 0.0f, 1.0f};
};

struct TeleportBeam {
	vec3 start = vec3(0.0f);
	vec3 end = vec3(0.0f);
	float startRadius = 0.0075f;
	float endRadius = 0.0075f;
	uint32_t color = 0xffffffff;
	bool visible = false;
};

struct TeleportRefinementDisk {
	vec3 center = vec3(0.0f);
	vec3 axisDepth = vec3(0.0f, 1.0f, 0.0f);
	vec3 axisHeight = vec3(0.0f, 0.0f, 1.0f);
	float radius = 1.75f;
	vec4 color = vec4(0.05f, 0.65f, 1.0f, 0.20f);
	bool visible = false;
};

struct CudaGlMappings{
	std::vector<CUgraphicsResource> resources;
	CUgraphicsResource resource;
	CUsurfObject surface;
	CUtexObject texture;
	
	
	bool isMapped = true;

	~CudaGlMappings(){
		if(isMapped){
			unmap();
		}
	}

	void unmap(){
		cuSurfObjectDestroy(surface);
		cuTexObjectDestroy (texture);
		cuGraphicsUnmapResources(resources.size(), resources.data(), ((CUstream)CU_STREAM_DEFAULT));
		// cuGraphicsUnregisterResource(resource);

		isMapped = false;
	};
};

struct SplatEditor{
	
	inline static SplatEditor* instance;

	Scene scene;
	shared_ptr<SNSplats> tempSplats = nullptr;
	shared_ptr<SNTriangles> sn_vr_editing = nullptr;
	shared_ptr<SNTriangles> sn_dbgsphere = nullptr;
	vector<shared_ptr<SNTriangles>> sn_teleport_avatar_parts;
	bool teleport_avatar_enabled = true;
	float teleport_avatar_scale = 0.8f; // Visual size only; does not change eye offset.
	vec3 teleport_avatar_forward = vec3(0.0f, 1.0f, 0.0f);
	shared_ptr<SNSplats> sn_brushsphere = nullptr;

	vector<SceneNode*> scheduledForRemoval;

	CudaModularProgram* prog_gaussians_rendering = nullptr;
	CudaModularProgram* prog_gaussians_editing = nullptr;
	CudaModularProgram* prog_points = nullptr;
	CudaModularProgram* prog_triangles = nullptr;
	CudaModularProgram* prog_lines = nullptr;
	CudaModularProgram* prog_helpers = nullptr;
	CudaModularProgram* prog_teleport_beam = nullptr;

	OpenVRHelper* ovr = nullptr;
	View viewLeft;
	View viewRight;
	shared_ptr<Framebuffer> fbGuiVr;
	shared_ptr<Framebuffer> fbGuiVr_assets;
	vec2 vrGuiResolution = {1024, 1024};
	MouseEvents mouse_prev;
	shared_ptr<ImguiNode> imn_brushes = nullptr;
	shared_ptr<ImguiNode> imn_assets = nullptr;
	shared_ptr<ImguiNode> imn_layers = nullptr;
	shared_ptr<ImguiNode> imn_painting = nullptr;
	shared_ptr<ImguiNode> imn_teleport = nullptr;


	vector<shared_ptr<Action>> history;
	int history_offset = 0;
	
	vector<GuiVrPage> vrPages = {
		{
			.label = "Brushes",
			.width = 440,
			.height = 700,
		},{
			.label = "Assets",
			.width = 440,
			.height = 700,
		},{
			.label = "Layers",
			.width = 440,
			.height = 700,
		}
	};
	int currentVrPage = 0;

	CUstream stream_upload;
	CUstream mainstream;
	CUstream sidestream;

	CUevent event_mainstream;
	CUevent event_edl_applied;
	CUevent ev_reset_stagecounters;

	// CUdeviceptr cptr_framebuffer;
	shared_ptr<CudaVirtualMemory> virt_framebuffer = CURuntime::allocVirtual("framebuffer");
	CUdeviceptr cptr_keys;
	CUdeviceptr cptr_lines = 0;
	CUdeviceptr cptr_numLines = 0;
	CUdeviceptr cptr_uniforms;

	DeviceState deviceState;
	void* h_state_pinned = nullptr;
	void* h_tilecounter = nullptr;
	CUdeviceptr cptr_state;

	Line* h_lines = nullptr;
	uint32_t* h_numLines = nullptr;
	shared_ptr<CudaVirtualMemory> virt_lines_host = CURuntime::allocVirtual("lines_host");
	CUdeviceptr cptr_numLines_host = 0;

	vector<TriangleQueueItem> triangleQueue;

	AssetLibrary assetLibrary;

	// This is prepared at the start of the frame and provides most properties for CUDA kernels
	CommonLaunchArgs launchArgs;

	shared_ptr<SNTriangles> sn_box = nullptr;

	ImGuiContext* imguicontext_desktop = nullptr;
	ImGuiContext* imguicontext_vr = nullptr;

	VRPanelMode active_vr_panel = VRPanelMode::PAINTING;
	VRPanelMode pending_vr_panel = VRPanelMode::PAINTING;
	bool vr_panel_switch_pending = false;

	//teleport
	CUdeviceptr cptr_teleport_best = 0;

	unsigned long long* h_teleport_best = nullptr;

	CUevent event_teleport_query_done;

	bool teleport_query_running = false;
	bool teleport_target_valid = false;
	NodeID teleport_environment_layer_id = -1;

	uint32_t teleport_target_index = 0;
	uint32_t teleport_ring_index = UINT32_MAX;
	vec3 teleport_target_position = vec3(0.0f);
	vec3 teleport_query_ray_origin = vec3(0.0f, 0.0f, 0.0f);
	vec3 teleport_query_ray_direction = vec3(0.0f, 1.0f, 0.0f);

	bool suppress_vr_gui_rendering = false;
	TeleportBeam teleport_beam;
	TeleportRefinementDisk teleport_refinement_disk;
	TeleportRefinementDisk teleport_refinement_proxy_disk;
	bool teleport_converging_guides_enabled = true;

	char teleport_participant_id[64] = "";
	char teleport_task_name[64] = "";
	char teleport_condition_name[64] = "";
	string teleport_log_filename = "";

	enum class TeleportMode : uint32_t {
		DIRECT_SPLAT = 0,
		REFINED_SPLAT = 1,
		SATOR_RAY_CURSOR = 2,
		INDICATOR_TELEPORT = 3,
		PLACE_INDICATOR = 4,
		ADAPTIVE_SPLAT = 5
	};

	enum class SatorDistanceControl : uint32_t {
		JOYSTICK = 0,
		TILT = 1
	};

	enum class TeleportQueryStage : uint32_t {
		NONE = 0,
		FIXED = 1,
		ADAPTIVE_NARROW = 2,
		ADAPTIVE_FALLBACK = 3
	};

	enum class TeleportPerfCondition : uint32_t {
		NONE = 0,
		RENDERING = 1,
		TARGET_SEARCH = 2,
		REFINEMENT = 3,
		CARRYING = 4,
		SATOR_CONTROL = 5
	};

	struct TeleportPerfMeasurement {
		bool enabled = false;
		bool renderingBaseline = false;

		string sceneName = "scene";
		uint32_t run = 1;

		TeleportPerfCondition frameCondition = TeleportPerfCondition::NONE;

		std::chrono::steady_clock::time_point previousFrameTime;
		bool hasPreviousFrameTime = false;

		vector<double> renderingFrameTimes;
		vector<double> targetSearchFrameTimes;
		vector<double> refinementFrameTimes;
		vector<double> carryingFrameTimes;
		vector<double> satorControlFrameTimes;
	};

	TeleportPerfMeasurement teleport_perf;

	struct TeleportRecord {
		uint32_t sequence = 0;
		double elapsedTime = 0.0;
		double intervalTime = 0.0;
		TeleportMode mode = TeleportMode::DIRECT_SPLAT;
		bool hasSatorControl = false;
		SatorDistanceControl satorDistanceControl = SatorDistanceControl::JOYSTICK;
		uint32_t gaussianIndex = std::numeric_limits<uint32_t>::max();

		bool hasGaussianInfo = false;

		vec3 gaussianLocalPosition = vec3(0.0f);
		vec3 gaussianWorldPosition = vec3(0.0f);
		vec3 gaussianScale = vec3(0.0f);
		vec3 targetPosition = vec3(0.0f);

		float opacity = -1.0f;
		float distance = 0.0f;
		float contentDistance = 0.0f;
		float cumulativeDistance = 0.0f;
		float cumulativeContentDistance = 0.0f;
		float routeLength = 0.0f;
		float overshoot = 0.0f;

		bool refinementUsed = false;
		float refinementDepth = 0.0f;
		float refinementHeight = 0.0f;
		vec3 contentAnchorPosition = vec3(0.0f);
	};

	struct TeleportEvalTarget {
		string name;
		Box3 bounds;
		vec3 aimPoint = vec3(0.0f);

		vec3 viewOrigin = vec3(0.0f);
		bool hasViewOrigin = false;

		vec3 planeViewA = vec3(0.0f);
		vec3 planeViewB = vec3(0.0f);
		vec3 planeViewC = vec3(0.0f);

		bool hasPlaneViewA = false;
		bool hasPlaneViewB = false;
		bool hasPlaneViewC = false;

		vector<vec3> viewOrigins;
	};

	TeleportEvalTarget teleport_eval_target;

	struct TeleportEvalConfig {
		int numViews = 180;
	};

	TeleportEvalConfig teleport_eval_config;

	struct TeleportEvalResult {
		enum class Type { CORRECT, OUTSIDE_TARGET, NO_HIT };

		Type type = Type::NO_HIT;
		uint32_t selectedIndex = std::numeric_limits<uint32_t>::max();
		float rayT = -1.0f;

		vec3 rayOrigin = vec3(0.0f);
		vec3 rayDirection = vec3(0.0f);
		vec3 gaussianWorldPosition = vec3(0.0f);
		vec3 projectedTarget = vec3(0.0f);
	};

	bool teleport_logging_enabled = true;       // 
	bool teleport_recording_enabled = false;    // 
	uint32_t teleport_count = 0;
	vector<TeleportRecord> teleport_records;
	float teleport_total_distance = 0.0f;
	float teleport_total_content_distance = 0.0f;
	float teleport_route_length = 0.0f;
	float teleport_overshoot = 0.0f;

	std::chrono::steady_clock::time_point teleport_recording_start_time;
	std::chrono::steady_clock::time_point teleport_last_record_time;
	bool teleport_has_last_record_time = false;

	struct TeleportIndicator {
		uint32_t id = 0;
		vec3 localPosition = vec3(0.0f);
		mat3 basisAtCreation = mat3(1.0f);
	};

	TeleportMode teleport_mode = TeleportMode::REFINED_SPLAT;
	std::vector<TeleportIndicator> teleport_indicators;
	uint32_t next_teleport_indicator_id = 0;
	int hovered_teleport_indicator = -1;
	float teleport_indicator_pick_radius = 0.15f;
	//for adaptive ray-cast
	float teleport_adaptive_max_end_radius = 0.15f;
	TeleportQueryStage teleport_query_stage = TeleportQueryStage::NONE;

	float teleport_fixed_radius = 0.05f;

	float teleport_adaptive_min_radius = 0.01f;
	float teleport_adaptive_expansion_start_distance = 2.0f;
	float teleport_adaptive_max_radius = 0.15f;
	// opacity to scale threshold
	float teleport_opacity_to_scale_threshold = 5.0f;
	bool teleport_show_opacity_scale_debug = false;
	bool teleport_show_opacity_scale_threshold = false;

	bool teleport_adaptive_last_used_fallback = false;
	float teleport_adaptive_selected_radius = 0.01f;
	//2 step refine
	bool teleport_refinement_active = false;
	uint32_t teleport_refinement_activation_count = 0;
	uint32_t teleport_refined_teleport_count = 0;

	vec3 teleport_content_anchor_position = vec3(0.0f);
	vec3 teleport_refinement_forward = vec3(0.0f, 1.0f, 0.0f);

	float teleport_refinement_radius = 1.45f;
	float teleport_refinement_activation_threshold = 0.12f;

	float teleport_refinement_depth = 0.0f;
	float teleport_refinement_height = 0.0f;

	shared_ptr<SNSplats> pendingImportedAsset = nullptr;
	bool pendingImportedAssetReady = false;
	float teleport_user_eye_height = 1.45f;

	// SATOR ray cursor
	SatorDistanceControl teleport_sator_distance_control = SatorDistanceControl::JOYSTICK;

	bool teleport_sator_active = false;
	bool teleport_sator_tilt_locked = false;

	float teleport_sator_initial_distance = 0.50f;
	float teleport_sator_cursor_distance = 0.50f;
	float teleport_sator_min_distance = 0.20f;
	float teleport_sator_max_distance = 21.52f;

	float teleport_sator_joystick_hold_time = 0.0f;

	vec3 teleport_sator_locked_origin = vec3(0.0f);
	vec3 teleport_sator_locked_direction = vec3(0.0f, 0.0f, -1.0f);

	float teleport_sator_locked_elevation = 0.0f;
	float teleport_sator_tilt_reference_distance = 0.50f;

	struct{
		float splatSize                  = 1.0f;
		int splatAppearance              = SPLATAPPEARANCE_GAUSSIAN;
		bool sort                        = true;
		bool disableCUDA                 = false;
		bool showSolid                   = false;
		bool showTiles                   = false;
		bool showRing                    = false;
		bool showHeatmap                 = false;
		bool showAxes                    = true;
		bool showGrid                    = true;
		bool makePoints                  = false;
		bool showBoundingBoxes           = false;
		bool frontToBack                 = true;
		bool enableOpenglRendering       = false;
		bool renderCooperative           = false;
		bool enableEDL                   = true;
		bool showSplatletBoxes           = false;
		float splatletBoxSizeThreshold   = 16.0f;
		bool showDirtySplatletBoxes      = false;
		int rendermode                   = RENDERMODE_COLOR;
		bool enableStereoFramebufferTest = false;
		bool enableSplatCulling          = false;
		bool disableFrustumCulling       = false;
		bool cullSmallSplats             = true;
		bool requestDebugDump            = false;
		bool enableOverlapped            = true;
		int splatRenderer                = SPLATRENDERER_3DGS;
		int intersectionMode             = INTERSECTION_APPROXIMATE;
		int brushColorMode               = BRUSHCOLORMODE_NORMAL;

		Brush brush;
		RectSelect rectselect;
		ColorCorrection colorCorrection;
		bool hideGUI                     = false;
		float vr_brushSize               = 0.1f;

		// - We need to be able to disable shortcuts while typing, for example.
		// - Actions at the end of a frame (e.g. processing an opened context menu) may want to disable shortcuts for as long as context menu is open.
		// - Such actions simply disable shortcuts for "two" frames. If the action is processed close to the end of the frame, -1 is subtracted right away, but the other preserves during the next frame. 
		int shortcutsDisabledForXFrames = 0;

		bool renderWarpwise = false;

		bool showDevStuff = false;
		bool showEditing = true;
		bool showKernelInfos = false;
		bool showMemoryInfos = false;
		bool showTimingInfos = false;
		bool showStats = false;
		bool showFileSaveDialog = false;
		bool showGettingStarted = false;
		bool showTianfang = false;
		bool showColorCorrection = false;
		bool showToolbar = true;
		bool openContextMenu = false;

		bool showInset = false;
		float dbg_factor = 1.0f;
		bool renderSoA = false;
		bool renderBandwidth = false;
		bool renderFragIntersections = false;

	} settings;

	struct {
		int hoveredObjectIndex = -1;
		int doubleClickedObjectIndex = -1;
		// int action = ACTION_NONE;
		// shared_ptr<SceneNode> placingItem = nullptr;
		int dbg_method = 0;

		shared_ptr<InputAction> currentAction = nullptr;
	} state;

	struct {
		bool menu_intersects = false;
		vec3 menu_intersection = vec3{0.0f, 0.0f, 0.0f};
	} state_vr; 

	struct ViewmodeDesktop{

	} viewmodeDesktop;

	struct ViewmodeDesktopVR{
		// Controllers should be relative to desktop monitor.
		// This transforms controllers into desktop view space.
		glm::mat4 m_controllers;

		// The right controllers neutral pose.
		// Use to callibrate from physical space to monitor/view space.
		glm::mat4 m_controller_neutral_left = glm::mat4(1.0);
		glm::mat4 m_controller_neutral_right = glm::mat4(1.0);
		glm::mat4 m_controller_neutral = glm::mat4(1.0);
	} viewmodeDesktopVr;

	struct ViewmodeImmersiveVR{
		// - Special world matrix for immersive VR.
		// - This is because in VR, we may want to transform the object
		//   as if it was a miniature toy model.
		// - Composition is: proj * view * world_vr * world
		glm::mat4 world_vr; 
	} viewmodeImmersiveVR;

	ImFont* font_default = nullptr;
	ImFont* font_big = nullptr;
	ImFont* font_vr_title = nullptr;
	ImFont* font_vr_text = nullptr;
	ImFont* font_vr_smalltext = nullptr;

	static void setup();

	void imguiStyleVR();

	CommonLaunchArgs getCommonLaunchArgs();

	// Some of these functions mainly exist because their functionality is used twice (e.g. via GUI & shortcut),
	// and we want to ensure that both approaches end up doing exactly the same thing.
	Box3 getSelectionAABB();
	void updateBoundingBox(SNSplats* node, bool onlySelected = false);
	void updateBoundingBox(SNTriangles* node);
	void updateBoundingBox(PointData& model);
	void insertNodeToNode(shared_ptr<SceneNode> node, shared_ptr<SceneNode> layer, bool onlySelected = false);
	bool merge(shared_ptr<SceneNode> snsource, shared_ptr<SceneNode> sntarget); 
	void applyTransformation(GaussianData& model, mat4 transformation, bool onlySelected = false);
	void apply(GaussianData& model, ColorCorrection value);
	// void setSelected(GaussianData& model);
	void setSelectedNode(SceneNode* node);
	void transformAllSelected(mat4 transform);
	void selectAll();
	void deselectAll();
	void invertSelection();
	void deleteSelection();
	void createOrUpdateThumbnail(SceneNode* node);
	void uploadSplats(SNSplats* node);
	void drawGUI();
	void resetEditor();
	void unloadTempSplats();
	void loadTempSplats(string path);
	void inputHandling();
	void inputHandlingDesktop();
	void inputHandlingVR();
	void setDesktopMode();
	void setDesktopVrMode();
	void setImmersiveVrMode();
	shared_ptr<SNSplats> clone(SNSplats* source);
	void sortSplatsDevice(SNSplats* node, bool putDeletedLast = false);
	void drawSphere(_DrawSphereArgs args);
	void drawTeleportBeam(vec3 start, vec3 end, float startRadius, float endRadius, vec4 color);
	void renderTeleportBeam(RenderTarget& target, CUstream stream);
	void drawTeleportRefinementDisk(vec3 center, vec3 axisDepth, vec3 axisHeight, float radius, vec4 color);
	void drawTeleportRefinementProxyWord(const string& text, vec3 center, vec3 axisX, vec3 axisY);
	void renderTeleportRefinementDisk(RenderTarget& target, CUstream stream);
	void drawLine(vec3 start, vec3 end, uint32_t color = 0xffff00ff);
	void drawBox(Box3 box, uint32_t color = 0xffff00ff);
	Uniforms getUniforms();
	void initCudaProgram();
	shared_ptr<SNTriangles> ovrToNode(string name, RenderModel_t* model, RenderModel_TextureMap_t* texture);
	shared_ptr<SceneNode> getSelectedNode();
	void temporarilyDisableShortcuts(int numFrames = 2);
	bool areShortcutsDisabled();
	void filter(SNSplats* source, SNSplats* target, FilterRules rules);
	shared_ptr<SNSplats> filterToNewLayer(FilterRules rules);
	vector<shared_ptr<SceneNode>> getLayers();
	// Erases given node at the very end of the frame. Was needed because there were issues when
	// directly deleting a node while drawing that node's gui.
	void scheduleRemoval(SceneNode* node);
	void applyDeletion();
	void revertDeletion();
	int32_t getNumSelectedSplats();
	int32_t getNumDeletedSplats();

	struct LambdaAction{
		function<void()> undo;
		function<void()> redo;
	};

	void addAction(LambdaAction action);
	void addAction(shared_ptr<Action> action);
	void undo();
	void redo();
	void clearHistory();

	// undoable functions
	void selectAll_undoable();
	void selectAllInNode_undoable(shared_ptr<SNSplats> node);
	void deselectAll_undoable();
	void invertSelection_undoable();
	void deleteSelection_undoable();
	void deleteNode_undoable(shared_ptr<SceneNode> node);
	shared_ptr<SNSplats> filterToNewLayer_undoable(FilterRules rules);
	shared_ptr<SNSplats> duplicateLayer_undoable(shared_ptr<SNSplats> node);
	// shared_ptr<SNSplats> extractLayer_undoable(shared_ptr<SNSplats> node);
	void merge_undoable(shared_ptr<SceneNode> snsource, shared_ptr<SceneNode> sntarget); 

	// GUI
	void setAction(shared_ptr<InputAction> action);
	void makeSettings();
	void makePerf();
	void makeMenubar();
	void makeLayers();
	void makeStats();
	void makeTODOs();
	void makeToolbar();
	// void makeGuiVr();
	void makeEditingGUI();
	void makeDevGUI();
	void makeDebugInfo();
	void makeAssetGUI();
	void makeColorCorrectionGui();
	void makeSaveFileGUI();
	void makeGettingStarted();

	// MISC
	CudaGlMappings mapCudaGl(shared_ptr<GLTexture> source);
	
	// UPDATE & DRAW 
	void update();
	void render();
	// void draw(Scene* scene, View view, RenderTarget& target);
	void draw(Scene* scene, vector<RenderTarget> targets);
	void postRenderStuff();

	//Tianfang 
	void setVRPanelMode(VRPanelMode mode);
	void setTeleportMode(TeleportMode mode);
	void requestVRPanelMode(VRPanelMode mode);
	void applyPendingVRPanelMode();
	void addTeleportIndicator(const vec3& worldPosition);
	bool saveTeleportIndicators(const string& path);
	bool loadTeleportIndicators(const string& path);
	void initializeTeleportEnvironmentLayer();
	SNSplats* getTeleportEnvironmentLayer();
	void resetTeleportEnvironmentLayer();

	void createAssetFromLayer(shared_ptr<SNSplats> node);
	void importAssetFromFile(const string& path);
	void finalizeImportedAsset();
	void insertLayerAboveEnvironment(shared_ptr<SceneNode> node);

	void setTeleportEvalTargetFromSelection(string name);
	void setTeleportEvalViewFromHMD();
	void runTeleportEvalRay(float radius);
	TeleportEvalResult runTeleportEvalRay(vec3 rayOrigin, vec3 rayDirection, float radius);
	void setTeleportEvalViewFromDesktopCamera();
	void generateTeleportEvalViewsFromDesktopCamera();
	void runTeleportEvalBatch(float radius);
	//multi view test
	void addTeleportEvalViewFromDesktopCamera();
	void clearTeleportEvalViews();
	bool showTeleportEvalDebug = false;
	bool showTeleportEvalAABB = true;
	bool showTeleportEvalViews = true;
	bool showTeleportEvalRays = true;
	float teleport_eval_debugCrossSize = 0.08f;

	void drawCross(vec3 position, float size, uint32_t color = 0xffff00ff);
	void drawTeleportEvalDebug();

	void setTeleportEvalPlaneViewAFromDesktopCamera();
	void setTeleportEvalPlaneViewBFromDesktopCamera();
	void setTeleportEvalPlaneViewCFromDesktopCamera();
	void generateTeleportEvalViewsFromPlane();
	void runTeleportEvalRadiusSweep(int minRadiusCm, int maxRadiusCm, int stepCm);
	//from reight controller
	bool getTeleportEvalRightControllerPositionInEnvironment(vec3& position);

	void setTeleportEvalPlaneViewAFromRightController();
	void setTeleportEvalPlaneViewBFromRightController();
	void setTeleportEvalPlaneViewCFromRightController();

	//void drawTeleportDisk(vec3 center, vec3 normal, float radius, vec4 fillColor, vec4 ringColor);
	void appendTeleportRecordToCSV(const TeleportRecord& record);

	void drawTeleportConvergingGuides(vec3 origin, vec3 rayDirection, vec3 target);

	//shared_ptr<SNSplats> importGaussianLayer(const string& path);
	//void importAssetFromFile(const string& path);

	SceneNode* getSelectedTransformLayer();
	void applyWorldTransformToSelectedLayer(const mat4& worldDelta);
	//for FPS
	void updateTeleportPerfFrame();
	void startTeleportPerfCapture(const string& sceneName);
	void stopTeleportPerfCapture();
	void exportTeleportPerfCSV();
	void clearTeleportPerf();
	bool hasTeleportCarryLayer();

	string sanitizeFilename(const string& input);
	float getTeleportContentScale();
};