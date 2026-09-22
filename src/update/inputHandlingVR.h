#pragma once

#include "actions/VR_TransformSelectionAction.h"

static ImguiNode* intersectedGuiNode = nullptr;
static ImguiNode* grabbedGuiNode = nullptr;
static inline shared_ptr<VR_TransformSelectionAction> transformSelectionAction = nullptr;

// Handle intersections of the controller with VR GUI panels.
// - Check intersection
// - Each backed by an imgui context. 
// - Injects controller state as imgui mouse input
//     - Intersection with gui panel = mouse pos
//     - Right controller trigger maps to left mouse button
void handleControllerImguiNodeIntersection(Controller controller){

	auto editor = SplatEditor::instance;
	auto ovr = editor->ovr;

	intersectedGuiNode = nullptr;
	editor->state_vr.menu_intersects = false;

	// Don't do menu stuff when we're currently draging a menu.
	if(grabbedGuiNode != nullptr) return;

	editor->scene.process<ImguiNode>([&](ImguiNode* guinode){
		if (!guinode || !guinode->page) {
			return;
		}

		ImGuiIO& panelIO = guinode->page->context->IO;

		// 
		panelIO.MouseDown[0] = false;
		panelIO.MouseWheel = 0.0f;

		// 
		if (!guinode->visible) {
			return;
		}
		mat4 mRot = glm::rotate(-140.0f * 3.1415f / 180.0f, vec3{ 1.0f, 0.0f, 0.0f });
		mat4 transform = mat4(ovr->flip * controller.transform) * mRot;

		vec3 origin = vec3(transform * vec4{0.0f, 0.0f, 0.0f, 1.0f});
		vec3 p1 = vec3(transform * vec4{0.0f, 1.0f, 0.0f, 1.0f});
		vec3 dir = normalize(p1 - origin);

		vec3 start = origin;
		vec3 end = origin + 2.0f * dir;
		
		vec3 plane_pos = vec3(guinode->transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 plane_N = vec3(guinode->transform * vec4(0.0f, 0.0f, 1.0f, 0.0f));
		float plane_D = -dot(plane_pos, plane_N);

		float d = ray_plane_intersection(origin, dir, plane_N, plane_D);
		vec3 I = origin + d * dir;

		bool intersectsMenuPlane = d > 0 && d != Infinity;

		if(intersectsMenuPlane){
			float sx = float(guinode->page->width) / 1000.0f;
			float sy = float(guinode->page->height) / 1000.0f;

			vec3 I_local = I - plane_pos;

			// 使用 w = 0，取得变换后的局部坐标轴。
			// 坐标轴长度本身就包含面板当前的实际缩放。
			vec3 rightAxis = vec3(guinode->transform * vec4(1.0f, 0.0f, 0.0f, 0.0f)
			);

			vec3 upAxis = vec3(guinode->transform * vec4(0.0f, 1.0f, 0.0f, 0.0f)
			);

			float panelHalfWidth = length(rightAxis);
			float panelHalfHeight = length(upAxis);

			if (panelHalfWidth < 0.000001f || panelHalfHeight < 0.000001f) {
				return;
			}

			vec3 right = rightAxis / panelHalfWidth;
			vec3 up = upAxis / panelHalfHeight;

			// 自动适配 0.12、0.10、0.08 等不同缩放
			float u = dot(right, I_local) / panelHalfWidth;

			float v = dot(up, I_local) / panelHalfHeight;

			bool intersectsMenuQuad = abs(u) < 1.0f && abs(v) < 1.0f;
			if(intersectsMenuQuad){

				if (controller.isMenuPressed() & !controller.wasMenuPressed()) {
					grabbedGuiNode = guinode;
					return;
				}

				editor->state_vr.menu_intersects = true;
				editor->state_vr.menu_intersection = I;
				mat4 t_sphere = glm::translate(I) * glm::scale(vec3{1.0f, 1.0f, 1.0f} * 0.005f);
				guinode->page->context->IO.MousePos = ImVec2(
					(u * 0.5f + 0.5f) * float(guinode->page->width), 
					float(guinode->page->height) - (v * 0.5f + 0.5f) * float(guinode->page->height)
				);

				if(controller.isTriggerPressed() && controller.valid){
					guinode->page->context->IO.MouseDown[0] = true;
				} else {
					guinode->page->context->IO.MouseDown[0] = false;
				}
				guinode->page->context->IO.MouseWheel = controller.state.rAxis[0].y;
				
				intersectedGuiNode = guinode;
			}

			end = I;
		}
	});
}



void SplatEditor::inputHandlingVR(){

	auto editor = SplatEditor::instance;
	Scene& scene = editor->scene;
	auto ovr = editor->ovr;
	auto& state = editor->state;
	for (auto& part : sn_teleport_avatar_parts) {
		part->visible = false;
	}

	#if !defined(NO_OPENVR)

	if(!ovr->isActive()){
		editor->sn_brushsphere->visible = false;
		return;
	}

	//if(transformSelectionAction == nullptr){
	//	transformSelectionAction = make_shared<VR_TransformSelectionAction>();
	//}

	ImGuiIO& io = ImGui::GetIO();

	// "reset" imgui inputs. Then map VR controller input to mouse input latter on
	io.MouseDown[0] = false;
	io.MousePos = ImVec2(0.0f, 0.0f);
	io.MouseWheel = 0.0f;

	Controller left = ovr->getLeftController();
	Controller right = ovr->getRightController();
	teleport_beam.visible = false;
	teleport_refinement_disk.visible = false;  //for refine disk
	teleport_refinement_proxy_disk.visible = false;
	bool teleportExecutedThisFrame = false;
	//bool skipTransformSelectionThisFrame = false;
	editor->initializeTeleportEnvironmentLayer();
	editor->drawTeleportEvalDebug();
	
	bool teleportRayValid = right.valid && !editor->state_vr.menu_intersects && state.currentAction == nullptr;
	vec3 teleportRayOrigin(0.0f);
	vec3 teleportRayDirection(0.0f, 0.0f, -1.0f);

	/*if (teleportRayValid) {
		const mat4 controllerPose = mat4(ovr->flip * right.transform);

		teleportRayOrigin = vec3(controllerPose * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		teleportRayDirection = normalize(vec3(controllerPose * vec4(0.0f, 0.0f, -1.0f, 0.0f)));

		if (right.isTriggerPressed()) {
			vec3 teleportRayEnd = teleportRayOrigin + teleportRayDirection * 20.0f;

			if (teleport_target_valid) {
				teleportRayEnd = teleport_target_position;
			}
			drawLine(teleportRayOrigin, teleportRayEnd, 0xffff0000);
		}
	}*/
	if (teleportRayValid) {
		const mat4 controllerPose = mat4(ovr->flip * right.transform);

		teleportRayOrigin = vec3(
			controllerPose *
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		teleportRayDirection = normalize(
			vec3(
				controllerPose *
				vec4(0.0f, 0.0f, -1.0f, 0.0f)
			)
		);
		// ============================================================
	// SATOR RAY CURSOR
	// Generates teleport_target_position without Gaussian queries.
	// ============================================================
		if (teleport_mode == TeleportMode::SATOR_RAY_CURSOR && right.isTriggerPressed()) {

			// Start a new SATOR target specification.
			if (!teleport_sator_active) {
				teleport_sator_active = true;
				teleport_sator_cursor_distance = teleport_sator_initial_distance;
				teleport_sator_joystick_hold_time = 0.0f;

				if (teleport_sator_distance_control == SatorDistanceControl::TILT) {
					teleport_sator_tilt_locked = true;
					teleport_sator_locked_origin = teleportRayOrigin;
					teleport_sator_locked_direction = teleportRayDirection;

					teleport_sator_locked_elevation =
						std::asin(glm::clamp(teleportRayDirection.z, -1.0f, 1.0f));

					teleport_sator_tilt_reference_distance =
						teleport_sator_cursor_distance;
				}
			}

			// --------------------------------------------------------
			// SATOR JOYSTICK
			// Simultaneous aiming + rate-based distance control.
			// --------------------------------------------------------
			if (teleport_sator_distance_control == SatorDistanceControl::JOYSTICK) {
				const float joystick = right.state.rAxis[0].y;
				const float deadzone = 0.15f;
				const float dt = float(GLRenderer::timeSinceLastFrame);

				if (std::abs(joystick) > deadzone) {
					teleport_sator_joystick_hold_time += dt;

					const float acceleration =
						glm::clamp(teleport_sator_joystick_hold_time / 2.5f, 0.0f, 1.0f);

					const float speed =
						glm::mix(0.47f, 11.62f, acceleration);

					teleport_sator_cursor_distance +=
						joystick * speed * dt;
				}
				else {
					teleport_sator_joystick_hold_time = 0.0f;
				}

				teleport_sator_cursor_distance =
					glm::clamp(
						teleport_sator_cursor_distance,
						teleport_sator_min_distance,
						teleport_sator_max_distance
					);

				teleport_target_position =
					teleportRayOrigin +
					teleportRayDirection *
					teleport_sator_cursor_distance;

				teleport_target_valid = true;
			}

			// --------------------------------------------------------
			// SATOR TILT
			// Ray is frozen when targeting begins.
			// Controller tilt only changes cursor depth.
			// --------------------------------------------------------
			else if (
				teleport_sator_distance_control == SatorDistanceControl::TILT &&
				teleport_sator_tilt_locked
				) {
				const float currentElevation = std::asin(glm::clamp(teleportRayDirection.z, -1.0f, 1.0f));

				float tiltDelta = currentElevation - teleport_sator_locked_elevation;

				const float maxTilt = glm::radians(40.0f);

				tiltDelta = glm::clamp(tiltDelta, -maxTilt, maxTilt);

				const float normalizedTilt = tiltDelta / maxTilt;

				// Upwards tilt -> farther away.
				if (normalizedTilt >= 0.0f) {
					teleport_sator_cursor_distance = teleport_sator_tilt_reference_distance + normalizedTilt * (teleport_sator_max_distance - teleport_sator_tilt_reference_distance);
				}
				// Downwards tilt -> closer to user.
				else {
					teleport_sator_cursor_distance = teleport_sator_tilt_reference_distance + normalizedTilt * (teleport_sator_tilt_reference_distance - teleport_sator_min_distance);
				}

				teleport_sator_cursor_distance = glm::clamp(teleport_sator_cursor_distance, teleport_sator_min_distance, teleport_sator_max_distance);

				teleport_target_position = teleport_sator_locked_origin + teleport_sator_locked_direction * teleport_sator_cursor_distance;

				teleport_target_valid = true;
			}

			if (sn_dbgsphere && teleport_target_valid) {
				sn_dbgsphere->visible = true;
				sn_dbgsphere->transform = glm::translate(teleport_target_position) * glm::scale(vec3(0.08f));
			}
		}


		if (right.isTriggerPressed()) {
			vec3 beamOrigin = teleportRayOrigin;
			vec3 beamDirection = teleportRayDirection;

			// In SATOR Tilt, the ray remains fixed after locking.
			if (
				teleport_mode == TeleportMode::SATOR_RAY_CURSOR &&
				teleport_sator_distance_control == SatorDistanceControl::TILT &&
				teleport_sator_tilt_locked
				) {
				beamOrigin = teleport_sator_locked_origin;
				beamDirection = teleport_sator_locked_direction;
			}

			vec3 teleportRayEnd =
				beamOrigin +
				beamDirection * 30.0f;

			// SATOR shows the complete fixed-length ray.
			// The sphere marks the movable cursor.
			if (teleport_mode == TeleportMode::SATOR_RAY_CURSOR) {
				teleportRayEnd =
					beamOrigin +
					beamDirection *
					teleport_sator_max_distance;
			}
			if (teleport_target_valid) {
				if (teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active) teleportRayEnd = teleport_content_anchor_position;
				else teleportRayEnd = teleport_target_position;
			}
			const vec3 teleportBeamStart = teleportRayOrigin + teleportRayDirection * 0.06f;
			const float beamRadius = 0.01f;
			const vec4 beamColor = teleport_target_valid ? vec4(0.05f, 0.65f, 1.0f, 1.0f) : vec4(0.10f, 0.40f, 1.0f, 1.0f);
			drawTeleportBeam(teleportBeamStart, teleportRayEnd, beamRadius, beamRadius, beamColor);
			if (teleport_converging_guides_enabled && teleport_target_valid && !teleport_refinement_active && teleport_mode != TeleportMode::SATOR_RAY_CURSOR) {
				drawTeleportConvergingGuides(teleportRayOrigin, teleportRayDirection, teleportRayEnd);
			}
		}
	}
	if (teleport_mode == TeleportMode::INDICATOR_TELEPORT && teleportRayValid && right.isTriggerPressed()) 
	{
		float bestT = std::numeric_limits<float>::max();
		int bestIndicator = -1;
		vec3 bestPosition = vec3(0.0f);
		SNSplats* indicatorEnvironment = editor->getTeleportEnvironmentLayer();

		const float radiusSquared = teleport_indicator_pick_radius * teleport_indicator_pick_radius;
		for (size_t i = 0; i < teleport_indicators.size(); ++i) {
			const TeleportIndicator& indicator = teleport_indicators[i];

			if (!indicatorEnvironment) {
				continue;
			}

			const mat4 indicatorTransform = scene.world->transform * indicatorEnvironment->transform;

			const vec3 position = vec3(
				indicatorTransform *
				vec4(indicator.localPosition, 1.0f)
			);

			const vec3 fromOrigin = position - teleportRayOrigin;
			const float t = dot(fromOrigin, teleportRayDirection);

			if (t < 0.20f || t > 20.0f) {
				continue;
			}

			const vec3 closestPoint = teleportRayOrigin + t * teleportRayDirection;

			const vec3 offset = position - closestPoint;
			const float distanceSquared = dot(offset, offset);

			if (distanceSquared > radiusSquared) {
				continue;
			}

			if (t < bestT) {
				bestT = t;
				bestIndicator = static_cast<int>(i);
				bestPosition = position;
			}
		}

		hovered_teleport_indicator = bestIndicator;

		if (bestIndicator >= 0) {
			teleport_target_position = bestPosition;
			teleport_target_valid = true;
		}
		else {
			teleport_target_valid = false;
		}
	}
	
	//const bool useDirectSplatQuery = teleport_mode == TeleportMode::DIRECT_SPLAT || teleport_mode == TeleportMode::PLACE_INDICATOR;
	//const bool useDirectSplatQuery =
	//	teleport_mode == TeleportMode::DIRECT_SPLAT ||
	//	teleport_mode == TeleportMode::GAUSSIAN_CENTER ||
	//	teleport_mode == TeleportMode::PLACE_INDICATOR;

	//if (teleportRayValid && useDirectSplatQuery && right.isTriggerPressed() && !teleport_query_running) {
	//	/*
	//	 * CUDA 查询和蓝色射线使用完全相同的
	//	 * origin 与 direction。
	//	 */
	//	vec3 rayOrigin = teleportRayOrigin;

	//	vec3 rayDirection = teleportRayDirection;

	//	// 保存本次异步查询使用的射线
	//	teleport_query_ray_origin = rayOrigin;

	//	teleport_query_ray_direction = rayDirection;

	//	// 将查询结果重置为 ULLONG_MAX
	//	cuMemsetD8Async(
	//		cptr_teleport_best,
	//		0xFF,
	//		sizeof(unsigned long long),
	//		sidestream
	//	);

	//	const float selectionRadius = 0.05f;
	//	const float minDistance = 0.20f;
	//	const float maxDistance = 30.0f;

	//	/*
	//	 * only use teleport in the original scene SNSplats。
	//	 */
	//	bool kernelLaunched = false;

	//	SNSplats* node = editor->getTeleportEnvironmentLayer();

	//	if (node && node->visible && node->dmng.data.count > 0) {
	//		GaussianData model = node->dmng.data;

	//		model.transform = scene.world->transform * node->transform;

	//		void* args[] = {
	//			&model,
	//			&rayOrigin,
	//			&rayDirection,
	//			(void*)&selectionRadius,
	//			(void*)&minDistance,
	//			(void*)&maxDistance,
	//			&cptr_teleport_best
	//		};

	//		prog_gaussians_editing->launch(
	//			"kernel_findTeleportPoint",
	//			args,
	//			model.count,
	//			sidestream
	//		);

	//		kernelLaunched = true;
	//	}

	//	if (kernelLaunched) {

	//		// 必须放在 kernel launch 后面，
	//		// 并使用同一个 sidestream
	//		cuMemcpyDtoHAsync(
	//			h_teleport_best,
	//			cptr_teleport_best,
	//			sizeof(unsigned long long),
	//			sidestream
	//		);

	//		cuEventRecord(
	//			event_teleport_query_done,
	//			sidestream
	//		);

	//		teleport_query_running = true;
	//	}
	//}
	const bool useDirectSplatQuery = teleport_mode == TeleportMode::DIRECT_SPLAT || teleport_mode == TeleportMode::REFINED_SPLAT || teleport_mode == TeleportMode::PLACE_INDICATOR || teleport_mode == TeleportMode::ADAPTIVE_SPLAT;
	const bool refinementFrozen = teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active;
	if (teleportRayValid && useDirectSplatQuery && right.isTriggerPressed() && !teleport_query_running && !refinementFrozen) {
		vec3 rayOrigin = teleportRayOrigin;
		vec3 rayDirection = teleportRayDirection;

		teleport_query_ray_origin = rayOrigin;
		teleport_query_ray_direction = rayDirection;

		cuMemsetD8Async(cptr_teleport_best, 0xFF, sizeof(unsigned long long), sidestream);

		float minDistance = 0.20f;
		float maxDistance = 30.0f;

		bool kernelLaunched = false;
		SNSplats* node = editor->getTeleportEnvironmentLayer();

		if (node && node->visible && node->dmng.data.count > 0) {
			GaussianData model = node->dmng.data;
			model.transform = scene.world->transform * node->transform;

			if (teleport_mode == TeleportMode::ADAPTIVE_SPLAT) {
				float selectionRadius = teleport_adaptive_min_radius;

				void* args[] = { &model, &rayOrigin, &rayDirection, &selectionRadius, &minDistance, &maxDistance, &cptr_teleport_best };
				prog_gaussians_editing->launch("kernel_findTeleportPoint", args, model.count, sidestream);

				teleport_query_stage = TeleportQueryStage::ADAPTIVE_NARROW;
			}
			else {
				float selectionRadius = teleport_fixed_radius;

				void* args[] = { &model, &rayOrigin, &rayDirection, &selectionRadius, &minDistance, &maxDistance, &cptr_teleport_best };
				prog_gaussians_editing->launch("kernel_findTeleportPoint", args, model.count, sidestream);

				teleport_query_stage = TeleportQueryStage::FIXED;
			}

			kernelLaunched = true;
		}

		if (kernelLaunched) {
			cuMemcpyDtoHAsync(h_teleport_best, cptr_teleport_best, sizeof(unsigned long long), sidestream);
			cuEventRecord(event_teleport_query_done, sidestream);
			teleport_query_running = true;
		}
	}

	// ============================================================
// TELEPORT: READ QUERY RESULT
// ============================================================
	//if (teleport_query_running) {

	//	CUresult queryResult = cuEventQuery(event_teleport_query_done);

	//	if (queryResult == CUDA_SUCCESS) {

	//		teleport_query_running = false;

	//		const unsigned long long packed = *h_teleport_best;

	//		const unsigned long long noResult = std::numeric_limits<unsigned long long>::max();

	//		if (packed == noResult) {

	//			teleport_target_valid = false;

	//			if (sn_dbgsphere) {
	//				sn_dbgsphere->visible = false;
	//			}

	//			/*println(
	//				"[Teleport] No point found."
	//			);*/
	//		}
	//		else {
	//			// lower 32 bits：point index
	//			teleport_target_index = static_cast<uint32_t>(packed & 0xffffffffull);

	//			// higher 32 bits：float bit pattern of t
	//			uint32_t tBits = static_cast<uint32_t>(packed >> 32);

	//			float rayT = 0.0f;

	//			std::memcpy(&rayT, &tBits, sizeof(float));

	//			teleport_target_position = teleport_query_ray_origin + rayT * teleport_query_ray_direction;

	//			teleport_target_valid = true;

	//			/*println(
	//				"[Teleport] index: {}, t: {}, "
	//				"position: {}, {}, {}",
	//				teleport_target_index,
	//				rayT,
	//				teleport_target_position.x,
	//				teleport_target_position.y,
	//				teleport_target_position.z
	//			);*/
	//			// 更新被选中 Gaussian 的 TELEPORT_TARGET flag
	//			/*if (teleport_ring_index != teleport_target_index) {
	//				const uint32_t previousIndex = teleport_ring_index;
	//				const uint32_t currentIndex = teleport_target_index;
	//				bool flagKernelLaunched = false;

	//				scene.process<SNSplats>([&](SNSplats* node) {
	//					if (flagKernelLaunched || !node || node->hidden || !node->visible) return;

	//					GaussianData model = node->dmng.data;
	//					if (model.count == 0) return;

	//					void* args[] = { &model, (void*)&previousIndex, (void*)&currentIndex };
	//					prog_gaussians_editing->launch("kernel_updateTeleportTargetFlag", args, 1, sidestream);

	//					flagKernelLaunched = true;
	//					});

	//				if (flagKernelLaunched) {
	//					teleport_ring_index = currentIndex;
	//				}
	//			}*/

	//			// VR 中的目标提示
	//			if (sn_dbgsphere) {
	//				sn_dbgsphere->visible = true;
	//				sn_dbgsphere->transform = glm::translate(teleport_target_position) * glm::scale(vec3(0.08f));
	//			}
	//		}
	//	}
	//	else if (
	//		queryResult != CUDA_ERROR_NOT_READY
	//		) {
	//		check(queryResult);
	//	}
	//}
	// ============================================================
// TELEPORT: READ QUERY RESULT
// ============================================================
	if (teleport_query_running) {
		CUresult queryResult = cuEventQuery(event_teleport_query_done);

		if (queryResult == CUDA_SUCCESS) {
			teleport_query_running = false;

			const unsigned long long packed = *h_teleport_best;
			const unsigned long long noResult = std::numeric_limits<unsigned long long>::max();

			if (packed == noResult && teleport_query_stage == TeleportQueryStage::ADAPTIVE_NARROW) {
				teleport_target_valid = false;
				if (sn_dbgsphere) sn_dbgsphere->visible = false;

				bool fallbackLaunched = false;
				SNSplats* node = editor->getTeleportEnvironmentLayer();

				if (node && node->visible && node->dmng.data.count > 0) {
					GaussianData model = node->dmng.data;
					model.transform = scene.world->transform * node->transform;

					vec3 rayOrigin = teleport_query_ray_origin;
					vec3 rayDirection = teleport_query_ray_direction;

					float minRadius = teleport_adaptive_min_radius;
					float expansionStartDistance = teleport_adaptive_expansion_start_distance;
					float maxRadius = teleport_adaptive_max_radius;
					float minDistance = 0.20f;
					float maxDistance = 30.0f;

					cuMemsetD8Async(cptr_teleport_best, 0xFF, sizeof(unsigned long long), sidestream);

					void* args[] = { &model, &rayOrigin, &rayDirection, &minRadius, &expansionStartDistance, &maxRadius, &minDistance, &maxDistance, &cptr_teleport_best };
					prog_gaussians_editing->launch("kernel_findTeleportPointAdaptiveFallback", args, model.count, sidestream);

					cuMemcpyDtoHAsync(h_teleport_best, cptr_teleport_best, sizeof(unsigned long long), sidestream);
					cuEventRecord(event_teleport_query_done, sidestream);

					teleport_query_stage = TeleportQueryStage::ADAPTIVE_FALLBACK;
					teleport_query_running = true;
					fallbackLaunched = true;
				}

				if (!fallbackLaunched) teleport_query_stage = TeleportQueryStage::NONE;
			}
			else if (packed == noResult) {
				teleport_target_valid = false;
				teleport_query_stage = TeleportQueryStage::NONE;

				if (sn_dbgsphere) sn_dbgsphere->visible = false;
			}
			else {
				teleport_target_index = static_cast<uint32_t>(packed & 0xffffffffull);

				uint32_t tBits = static_cast<uint32_t>(packed >> 32);
				float rayT = 0.0f;
				std::memcpy(&rayT, &tBits, sizeof(float));

				//teleport_target_position = teleport_query_ray_origin + rayT * teleport_query_ray_direction;
				//teleport_target_valid = true;
				// add refinement
				const vec3 primaryTarget = teleport_query_ray_origin + rayT * teleport_query_ray_direction;

				teleport_target_position = primaryTarget;
				teleport_target_valid = true;

				if (teleport_mode == TeleportMode::REFINED_SPLAT && !teleport_refinement_active) {
					teleport_content_anchor_position = primaryTarget;

					vec3 horizontalDirection(
						teleport_query_ray_direction.x,
						teleport_query_ray_direction.y,
						0.0f
					);

					if (glm::length(horizontalDirection) > 0.0001f) teleport_refinement_forward = glm::normalize(horizontalDirection);

					teleport_refinement_depth = 0.0f;
					teleport_refinement_height = 0.0f;
				}

				if (teleport_query_stage == TeleportQueryStage::ADAPTIVE_NARROW) {
					teleport_adaptive_last_used_fallback = false;
					teleport_adaptive_selected_radius = teleport_adaptive_min_radius;
				}
				else if (teleport_query_stage == TeleportQueryStage::ADAPTIVE_FALLBACK) {
					teleport_adaptive_last_used_fallback = true;

					float startDistance = glm::max(teleport_adaptive_expansion_start_distance, 0.20f);
					float u = rayT > startDistance ? (rayT - startDistance) / (30.0f - startDistance) : 0.0f;
					u = glm::clamp(u, 0.0f, 1.0f);

					teleport_adaptive_selected_radius = teleport_adaptive_min_radius + u * (teleport_adaptive_max_radius - teleport_adaptive_min_radius);
				}

				if (teleport_mode == TeleportMode::ADAPTIVE_SPLAT) {
					println("[Adaptive Teleport] fallback={} t={} radius={} cm index={}", teleport_adaptive_last_used_fallback, rayT, teleport_adaptive_selected_radius * 100.0f, teleport_target_index);
				}

				teleport_query_stage = TeleportQueryStage::NONE;

				if (sn_dbgsphere) {
					const float contentScale = getTeleportContentScale();
					const float markerScale = glm::max(0.08f * contentScale, 0.02f);
					sn_dbgsphere->visible = true;
					sn_dbgsphere->transform = glm::translate(teleport_target_position) * glm::scale(vec3(markerScale));
				}
			}
		}
		else if (queryResult != CUDA_ERROR_NOT_READY) {
			check(queryResult);
		}
	}

	// ============================================================
// TELEPORT: OPTIONAL 2D DESTINATION REFINEMENT
// Left touchpad is temporarily owned by teleport while
// the right trigger is held.
// ============================================================
	if (teleport_mode == TeleportMode::REFINED_SPLAT && right.valid && left.valid && right.isTriggerPressed() && teleport_target_valid) {
		vec2 touch(left.state.rAxis[0].x, left.state.rAxis[0].y);
		float touchLength = glm::length(touch);

		if (!teleport_refinement_active && touchLength > teleport_refinement_activation_threshold) {
			teleport_refinement_active = true;
			teleport_refinement_activation_count++;

			teleport_content_anchor_position = teleport_target_position;

			vec3 horizontalDirection(
				teleport_query_ray_direction.x,
				teleport_query_ray_direction.y,
				0.0f
			);

			if (glm::length(horizontalDirection) > 0.0001f) teleport_refinement_forward = glm::normalize(horizontalDirection);

			println(
				"[Teleport Refinement] activated count={}",
				teleport_refinement_activation_count
			);
		}

		if (teleport_refinement_active && touchLength > teleport_refinement_activation_threshold) {
			if (touchLength > 1.0f) touch /= touchLength;

			const float contentScale = getTeleportContentScale();
			const float scaledRefinementRadius = teleport_refinement_radius * contentScale;

			teleport_refinement_depth = touch.x * scaledRefinementRadius;
			teleport_refinement_height = touch.y * scaledRefinementRadius;

			const vec3 up(0.0f, 0.0f, 1.0f);

			teleport_target_position = teleport_content_anchor_position + teleport_refinement_depth * teleport_refinement_forward + teleport_refinement_height * up;
			
			if (sn_dbgsphere) {
				const float markerScale = glm::max(0.08f * contentScale, 0.02f);
				sn_dbgsphere->visible = true;
				sn_dbgsphere->transform = glm::translate(teleport_target_position) * glm::scale(vec3(markerScale));
			}
		}
	}
	/*if (teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active && teleport_target_valid && right.isTriggerPressed()) {
		const vec3 up(0.0f, 0.0f, 1.0f);
		const vec4 diskColor(0.05f, 0.65f, 1.0f, 0.80f);

		drawTeleportRefinementDisk(
			teleport_content_anchor_position,
			teleport_refinement_forward,
			up,
			teleport_refinement_radius,
			diskColor
		);
	}*/
	if (teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active && teleport_target_valid && right.isTriggerPressed()) {
		const vec3 up(0.0f, 0.0f, 1.0f);
		const vec4 diskColor(0.05f, 0.65f, 1.0f, 0.80f);
		const float contentScale = getTeleportContentScale();
		const float scaledRefinementRadius = teleport_refinement_radius * contentScale;

		drawTeleportRefinementDisk(teleport_content_anchor_position, teleport_refinement_forward, up, scaledRefinementRadius, diskColor);

		// Right-controller-proximal refinement preview.
		if (right.valid) {
			const mat4 rightPose = mat4(ovr->flip * right.transform);

			// Mirrored from the previous left-controller placement.
			const vec3 proxyCenter = vec3(rightPose * vec4(0.10f, 0.09f, -0.12f, 1.0f));

			// Make the proxy disk face the HMD.
			const vec3 hmdPosition = vec3(ovr->flip * ovr->getHmdPose() * vec4(0.0f, 0.0f, 0.0f, 1.0f));

			vec3 toHmd = hmdPosition - proxyCenter;

			if (glm::length(toHmd) > 0.0001f) {
				toHmd = glm::normalize(toHmd);

				vec3 proxyDepth = glm::cross(up, toHmd);
				if (glm::length(proxyDepth) < 0.0001f) proxyDepth = vec3(1.0f, 0.0f, 0.0f);
				else proxyDepth = glm::normalize(proxyDepth);

				vec3 proxyHeight = glm::normalize(glm::cross(toHmd, proxyDepth));

				const float proxyRadius = 0.06f;

				teleport_refinement_proxy_disk.center = proxyCenter;
				teleport_refinement_proxy_disk.axisDepth = proxyDepth;
				teleport_refinement_proxy_disk.axisHeight = proxyHeight;
				teleport_refinement_proxy_disk.radius = proxyRadius;
				teleport_refinement_proxy_disk.color = diskColor;
				teleport_refinement_proxy_disk.visible = true;

				const float normalizedDepth = glm::clamp(teleport_refinement_depth / scaledRefinementRadius, -1.0f, 1.0f);
				const float normalizedHeight = glm::clamp(teleport_refinement_height / scaledRefinementRadius, -1.0f, 1.0f);

				const vec3 proxyTarget = proxyCenter + normalizedDepth * proxyRadius * proxyDepth + normalizedHeight * proxyRadius * proxyHeight;

				drawSphere({ .pos = proxyTarget, .scale = vec3(0.012f), .color = vec4(1.0f, 0.15f, 0.10f, 1.0f) });

				drawTeleportRefinementProxyWord("BACK", proxyCenter - (proxyRadius + 0.045f) * proxyDepth, proxyDepth, proxyHeight);
				drawTeleportRefinementProxyWord("FRONT", proxyCenter + (proxyRadius + 0.045f) * proxyDepth, proxyDepth, proxyHeight);
				drawTeleportRefinementProxyWord("DOWN", proxyCenter - (proxyRadius + 0.040f) * proxyHeight, proxyDepth, proxyHeight);
				drawTeleportRefinementProxyWord("UP", proxyCenter + (proxyRadius + 0.040f) * proxyHeight, proxyDepth, proxyHeight);
			}
		}
	}
	// Preview the destination eye position after all target/refinement updates.
	// Keep the existing sphere at the content target; the avatar marks the viewpoint.
	if (teleport_avatar_enabled && !sn_teleport_avatar_parts.empty() && teleportRayValid && right.isTriggerPressed() && teleport_target_valid && teleport_mode != TeleportMode::PLACE_INDICATOR) {
		const mat4 hmdPose = mat4(ovr->flip * ovr->getHmdPose());
		const vec3 hmdPosition = vec3(hmdPose * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		const vec3 up(0.0f, 0.0f, 1.0f);
		const float contentScale = getTeleportContentScale();
		vec3 destinationEye = teleport_target_position;

		if (teleport_mode == TeleportMode::SATOR_RAY_CURSOR) {
			const mat4 controllerPose = mat4(ovr->flip * right.transform);
			const vec3 controllerPosition = vec3(controllerPose * vec4(0.0f, 0.0f, 0.0f, 1.0f));
			destinationEye += hmdPosition - controllerPosition;
		}
		else if (!(teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active)) {
			destinationEye += up * (teleport_user_eye_height * contentScale);
		}

		vec3 forward = vec3(hmdPose * vec4(0.0f, 0.0f, -1.0f, 0.0f));
		forward.z = 0.0f;
		if (glm::length(forward) > 0.0001f) teleport_avatar_forward = glm::normalize(forward);
		forward = teleport_avatar_forward;
		const vec3 avatarRight = glm::normalize(glm::cross(forward, up));

		// Maps model +Y to scene +Z and model -Z to horizontal HMD forward.
		mat4 orientation(1.0f);
		orientation[0] = vec4(avatarRight, 0.0f);
		orientation[1] = vec4(up, 0.0f);
		orientation[2] = vec4(-forward, 0.0f);
		const float avatarScale = teleport_avatar_scale * contentScale;
		if (avatarScale > 0.0f) {
			const mat4 avatarTransform = glm::translate(destinationEye) * orientation * glm::scale(vec3(avatarScale));

			for (auto& part : sn_teleport_avatar_parts) 
			{
				part->transform = avatarTransform;
				part->visible = true;
			}
		}
	}
	if (teleport_perf.enabled) {
		TeleportPerfCondition condition = TeleportPerfCondition::NONE;

		if (teleport_perf.renderingBaseline) {
			condition = TeleportPerfCondition::RENDERING;
		}
		else if (right.valid && right.isTriggerPressed() && !editor->state_vr.menu_intersects && state.currentAction == nullptr) {
			const bool contentTeleport =
				teleport_mode == TeleportMode::DIRECT_SPLAT ||
				teleport_mode == TeleportMode::REFINED_SPLAT ||
				teleport_mode == TeleportMode::ADAPTIVE_SPLAT;

			const bool carrying = right.isMenuPressed() && grabbedGuiNode == nullptr && hasTeleportCarryLayer();

			if (carrying) {
				condition = TeleportPerfCondition::CARRYING;
			}
			else if (teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active) {
				condition = TeleportPerfCondition::REFINEMENT;
			}
			else if (teleport_mode == TeleportMode::SATOR_RAY_CURSOR) {
				condition = TeleportPerfCondition::SATOR_CONTROL;
			}
			else if (contentTeleport) {
				condition = TeleportPerfCondition::TARGET_SEARCH;
			}
		}

		teleport_perf.frameCondition = condition;
	}
	// ============================================================
// TELEPORT: EXECUTE ON TRIGGER RELEASE
// ============================================================
	if (right.valid && !right.isTriggerPressed() && teleport_target_valid && !teleport_query_running && !editor->state_vr.menu_intersects && state.currentAction == nullptr)
	{
		if (teleport_mode == TeleportMode::PLACE_INDICATOR) {
			addTeleportIndicator(teleport_target_position);

			teleport_target_valid = false;

			if (sn_dbgsphere) {
				sn_dbgsphere->visible = false;
			}
		}
		else if (teleport_mode == TeleportMode::DIRECT_SPLAT || teleport_mode == TeleportMode::REFINED_SPLAT || teleport_mode == TeleportMode::SATOR_RAY_CURSOR || teleport_mode == TeleportMode::INDICATOR_TELEPORT || teleport_mode == TeleportMode::ADAPTIVE_SPLAT)
		{
			vec3 hmdPosition = vec3(ovr->flip * ovr->getHmdPose() * vec4(0.0f, 0.0f, 0.0f, 1.0f));

			const mat4 rightControllerPose = mat4(ovr->flip * right.transform);
			const vec3 rightControllerPosition = vec3(rightControllerPose * vec4(0.0f, 0.0f, 0.0f, 1.0f));

			//vec3 delta = hmdPosition - teleport_target_position;

			//const bool use3DRefinedDestination = teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active;

			//if (!use3DRefinedDestination) {
			//	//const float controllerToHmdHeight = hmdPosition.z - rightControllerPosition.z;
			//	//const float desiredHmdHeight = teleport_target_position.z + controllerToHmdHeight;

			//	//delta.z = hmdPosition.z - desiredHmdHeight;
			//	delta.z = hmdPosition.z - teleport_user_eye_height - teleport_target_position.z;
			//}
			vec3 delta;
			if (teleport_mode == TeleportMode::SATOR_RAY_CURSOR) {
				delta = rightControllerPosition - teleport_target_position;
			}
			else {
				delta = hmdPosition - teleport_target_position;
				const bool use3DRefinedDestination = teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active;
				if (!use3DRefinedDestination) {
					const float contentScale = getTeleportContentScale();
					const float scaledEyeHeight = teleport_user_eye_height * contentScale;
					delta.z = hmdPosition.z - scaledEyeHeight - teleport_target_position.z;
				}
			}
			if (teleport_recording_enabled) {
				TeleportRecord record;

				const auto now = std::chrono::steady_clock::now();

				record.sequence = teleport_count + 1;
				record.elapsedTime = std::chrono::duration<double>(
					now - teleport_recording_start_time
				).count();

				if (teleport_has_last_record_time) {
					record.intervalTime = std::chrono::duration<double>(
						now - teleport_last_record_time
					).count();
				}
				else {
					record.intervalTime = 0.0;
				}

				teleport_last_record_time = now;
				teleport_has_last_record_time = true;

				record.mode = teleport_mode;
				record.targetPosition = teleport_target_position;
				record.distance = glm::length(delta);
				const float contentScale = glm::max(getTeleportContentScale(), 0.000001f);
				record.contentDistance = record.distance / contentScale;
				record.hasSatorControl = teleport_mode == TeleportMode::SATOR_RAY_CURSOR;

				if (record.hasSatorControl) {
					record.satorDistanceControl = teleport_sator_distance_control;
				}

				record.refinementUsed = teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active;
				record.refinementDepth = record.refinementUsed ? teleport_refinement_depth : 0.0f;
				record.refinementHeight = record.refinementUsed ? teleport_refinement_height : 0.0f;
				record.contentAnchorPosition = record.refinementUsed ? teleport_content_anchor_position : teleport_target_position;

				if (teleport_mode == TeleportMode::DIRECT_SPLAT || teleport_mode == TeleportMode::REFINED_SPLAT || teleport_mode == TeleportMode::ADAPTIVE_SPLAT) {
					SNSplats* environment = editor->getTeleportEnvironmentLayer();

					if (environment && teleport_target_index < environment->dmng.data.count) {
						GaussianData model = environment->dmng.data;

						vec3 gaussianLocalPosition;
						vec3 gaussianScale;
						Color gaussianColor;

						CURuntime::check(cuMemcpyDtoH(
							&gaussianLocalPosition,
							(CUdeviceptr)model.position + sizeof(vec3) * teleport_target_index,
							sizeof(vec3)
						));

						CURuntime::check(cuMemcpyDtoH(
							&gaussianScale,
							(CUdeviceptr)model.scale + sizeof(vec3) * teleport_target_index,
							sizeof(vec3)
						));

						CURuntime::check(cuMemcpyDtoH(
							&gaussianColor,
							(CUdeviceptr)model.color + sizeof(Color) * teleport_target_index,
							sizeof(Color)
						));

						const mat4 gaussianTransform = scene.world->transform * environment->transform;
						const vec3 gaussianWorldPosition = vec3(
							gaussianTransform *
							vec4(gaussianLocalPosition, 1.0f)
						);

						const vec4 normalizedColor = gaussianColor.normalized();

						record.gaussianIndex = teleport_target_index;
						record.gaussianLocalPosition = gaussianLocalPosition;
						record.gaussianWorldPosition = gaussianWorldPosition;
						record.gaussianScale = gaussianScale;
						record.opacity = normalizedColor.a;
						record.hasGaussianInfo = true;
					}
				}

				teleport_count++;
				teleport_total_distance += record.distance;
				teleport_total_content_distance += record.contentDistance;

				record.cumulativeDistance = teleport_total_distance;
				record.cumulativeContentDistance = teleport_total_content_distance;
				record.routeLength = teleport_route_length;
				record.overshoot = teleport_total_content_distance - teleport_route_length;

				teleport_overshoot = record.overshoot;
				teleport_records.push_back(record);
				appendTeleportRecordToCSV(record);

				println(
					"[Teleport Log] #{} gaussian={} "
					"gaussianLocal=({}, {}, {}) "
					"gaussianWorld=({}, {}, {}) "
					"scale=({}, {}, {}) "
					"target=({}, {}, {}) "
					"opacity={} distance={}",
					record.sequence,
					record.gaussianIndex,
					record.gaussianLocalPosition.x,
					record.gaussianLocalPosition.y,
					record.gaussianLocalPosition.z,
					record.gaussianWorldPosition.x,
					record.gaussianWorldPosition.y,
					record.gaussianWorldPosition.z,
					record.gaussianScale.x,
					record.gaussianScale.y,
					record.gaussianScale.z,
					record.targetPosition.x,
					record.targetPosition.y,
					record.targetPosition.z,
					record.opacity,
					record.distance
				);
			}
			shared_ptr<SNSplats> carriedLayer = nullptr;

			if (right.isMenuPressed() && grabbedGuiNode == nullptr) {
				for (auto& child : scene.world->children) {
					auto splatNode = dynamic_pointer_cast<SNSplats>(child);

					if (!splatNode || !splatNode->selected) {
						continue;
					}

					if (splatNode->ID == teleport_environment_layer_id) {
						continue;
					}

					carriedLayer = splatNode;
					break;
				}
			}

			/*
			 * 先结束旧的抓取动作。
			 * 必须在修改 world 和 layer transform 之前执行，
			 * 使旧 Action 在一致的坐标状态中结束。
			 */
			const mat4 oldWorldTransform = scene.world->transform;

			mat4 carriedGlobalBefore(1.0f);

			if (carriedLayer) {
				carriedGlobalBefore = oldWorldTransform * carriedLayer->transform;
			}

			const mat4 newWorldTransform = glm::translate(delta) * oldWorldTransform;

			scene.world->transform = newWorldTransform;

			/*if (carriedLayer) {
				carriedLayer->transform = glm::inverse(newWorldTransform) * carriedGlobalBefore;
			}*/
			if (carriedLayer) {
				carriedLayer->transform = glm::inverse(scene.world->transform) * carriedGlobalBefore;
				teleportExecutedThisFrame = true;
			}
			if (teleport_mode == TeleportMode::REFINED_SPLAT && teleport_refinement_active) {
				teleport_refined_teleport_count++;

				println(
					"[Teleport Refinement] executed, refinedTeleports={}",
					teleport_refined_teleport_count
				);
			}


			teleport_target_valid = false;
			// for refine
			teleport_refinement_active = false;
			teleport_refinement_depth = 0.0f;
			teleport_refinement_height = 0.0f;
			teleport_sator_active = false;
			teleport_sator_tilt_locked = false;
			teleport_sator_cursor_distance = teleport_sator_initial_distance;
			teleport_sator_joystick_hold_time = 0.0f;

			if (sn_dbgsphere) {
				sn_dbgsphere->visible = false;
			}

			/*println(
				"[Teleport] executed."
			);*/
		}
	}
	if (left.valid) {

		// 根据当前页面选择唯一需要显示和更新的面板
		shared_ptr<ImguiNode> activePanel = nullptr;
		float panelScale = 0.12f;

		switch (active_vr_panel) {

		case VRPanelMode::PAINTING:
			activePanel = imn_painting;
			panelScale = 0.12f;
			break;

		case VRPanelMode::BRUSHES:
			activePanel = imn_brushes;
			panelScale = 0.12f;
			break;

		case VRPanelMode::LAYERS:
			activePanel = imn_layers;
			panelScale = 0.10f;
			break;

		case VRPanelMode::ASSETS:
			activePanel = imn_assets;
			panelScale = 0.08f;
			break;

		case VRPanelMode::TELEPORT:
			activePanel = imn_teleport;
			panelScale = 0.12f;
			break;

		default:
			break;
		}

		if (activePanel && activePanel->page) {

			const mat4 controllerTransform = mat4(ovr->flip * left.transform);

			const vec3 panelOffset(
				0.05f,
				0.08f,
				-0.10f
			);

			const float sx = static_cast<float>(activePanel->page->width) / 1000.0f;
			const float sy = static_cast<float>(activePanel->page->height) / 1000.0f;

			const mat4 mOffset = glm::translate(panelOffset);
			const mat4 mRot = glm::rotate(-0.5f * 3.1415926f, vec3(1.0f, 0.0f, 0.0f));
			const mat4 mScale = glm::scale(vec3(sx, sy, 1.0f) * panelScale);

			activePanel->transform = controllerTransform * mOffset * mRot * mScale;
		}
	}
	// TRANSFORM IMGUI NODE
	if (teleportExecutedThisFrame) {
		/*
		 * Teleport 当帧保留当前抓取 Action，
		 * 但不调用 update()，也不销毁它。
		 */
	}
	else if (grabbedGuiNode && right.isMenuPressed()) {
		transformSelectionAction = nullptr;

		float sx = float(grabbedGuiNode->page->width) / 1000.0f;
		float sy = float(grabbedGuiNode->page->height) / 1000.0f;

		mat4 mRot = glm::rotate(-3.1415f * 0.5f, vec3{ 1.0f, 0.0f, 0.0f });
		mat4 mScale = glm::scale(vec3{sx, sy, 1.0f} * 0.2f);

		grabbedGuiNode->transform = mat4(ovr->flip * right.transform) * mRot * mScale;
	} else if (grabbedGuiNode && !right.isMenuPressed()) {
		transformSelectionAction = nullptr;

		grabbedGuiNode = nullptr;
	}else if(right.isMenuPressed()){

		if(transformSelectionAction == nullptr){
			transformSelectionAction = make_shared<VR_TransformSelectionAction>();
		}

		transformSelectionAction->update();

	}else{
		transformSelectionAction = nullptr;
	}

	// CONTROLLER-MENU INTERSECTION
	handleControllerImguiNodeIntersection(right);

	// Update current action, or cancel with left trigger
	// Actions typically utilize the right trigger buttons.
	if(state.currentAction){
		if(left.triggered()){
			editor->setAction(nullptr);
		}else{
			state.currentAction->update();
		}
	}

	{ // UNDO / REDO HANDLING with left touchpad
		float value = left.state.rAxis[0].x;

		constexpr int STATE_NEUTRAL = 0;
		constexpr int STATE_LEFT = 1;
		constexpr int STATE_RIGHT = 2;
		constexpr float THRESHOLD_LEFT_RIGHT = 0.7f;
		constexpr float THRESHOLD_NEUTRAL = 0.1f;

		static int undoRedoState = STATE_NEUTRAL;
		static bool suppressUndoRedoUntilNeutral = false;

		const bool teleportOwnsLeftTouchpad =
			teleport_mode == TeleportMode::REFINED_SPLAT &&
			right.valid &&
			right.isTriggerPressed();

		if (teleportOwnsLeftTouchpad) {
			suppressUndoRedoUntilNeutral = true;
			undoRedoState = STATE_NEUTRAL;
		}
		else if (suppressUndoRedoUntilNeutral) {
			if (std::abs(value) < THRESHOLD_NEUTRAL) {
				suppressUndoRedoUntilNeutral = false;
				undoRedoState = STATE_NEUTRAL;
			}
		}
		else {
			int newState = STATE_NEUTRAL;
			if (value < -THRESHOLD_LEFT_RIGHT) newState = STATE_LEFT;
			else if (value > THRESHOLD_LEFT_RIGHT) newState = STATE_RIGHT;

			if (undoRedoState == STATE_NEUTRAL && newState == STATE_LEFT) editor->undo();
			else if (undoRedoState == STATE_NEUTRAL && newState == STATE_RIGHT) editor->redo();

			undoRedoState = newState;
		}
	}

	// Transform "world" with A button, which unfortunately is equal to grip button in OpenVR's legacy input handling.
	if(left.isGripPressed() && right.isGripPressed()){
		// GRIP SCALE ROTATE TRANSLATE

		vec3 left_pos = vec3(ovr->flip * left.transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 left_previousPos = vec3(ovr->flip * left.previous.transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 left_delta = left_pos - left_previousPos;

		vec3 right_pos = vec3(ovr->flip * right.transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 right_previousPos = vec3(ovr->flip * right.previous.transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 right_delta = right_pos - right_previousPos;

		float amount = length(left_pos - right_pos);
		float amountPrevious = length(left_previousPos - right_previousPos);
		float scale = amount / amountPrevious;

		vec3 center = (left_pos + right_pos) * 0.5f;
		vec3 center_prev = (left_previousPos + right_previousPos) * 0.5f;

		vec2 dirxy = normalize(vec2((right_pos - left_pos)));
		vec2 dirxy_prev = normalize(vec2((right_previousPos - left_previousPos)));
		

		float angle_prev = atan2(dirxy_prev.y, dirxy_prev.x);
		float angle = atan2(dirxy.y, dirxy.x);
		float angle_diff = angle - angle_prev;

		mat4 rot = rotate(angle_diff, vec3{0.0f, 0.0f, 1.0f});
		mat4 trans = translate(center - center_prev);
		
		mat4 transform = trans * translate(center) * rot * glm::scale(vec3{ scale, scale, scale }) * translate(-center);

		//scene.world->transform = transform * scene.world->transform;
		SceneNode* selectedLayer = getSelectedTransformLayer();

		if (selectedLayer) {
			applyWorldTransformToSelectedLayer(transform);
		}
		else {
			scene.world->transform = transform * scene.world->transform;
		}
		const float worldScale = glm::length(vec3(scene.world->transform[0]));
		const float layerScale = selectedLayer ? glm::length(vec3(selectedLayer->transform[0])) : 1.0f;

		println("[Scale] selectedLayer={} frameScale={} worldScale={} layerScale={}", selectedLayer ? 1 : 0, scale, worldScale, layerScale);

	}else if(right.isGripPressed()){
		// GRIP TRANSLATE

		vec3 right_pos = vec3(ovr->flip * right.transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 right_previousPos = vec3(ovr->flip * right.previous.transform * vec4(0.0f, 0.0f, 0.0f, 1.0f));
		vec3 right_delta = right_pos - right_previousPos;

		scene.world->transform = translate(right_delta) * scene.world->transform;
	}

	// FLY-THROUGH USING RIGHT JOYSTICK
	//if(!editor->state_vr.menu_intersects){ 
	const bool teleportTargetingActive = right.valid && right.isTriggerPressed() && (teleport_mode == TeleportMode::REFINED_SPLAT || teleport_mode == TeleportMode::SATOR_RAY_CURSOR);
	if (!editor->state_vr.menu_intersects && !teleportTargetingActive) {

		mat4 mRot = glm::rotate(-140.0f * 3.1415f / 180.0f, vec3{ 1.0f, 0.0f, 0.0f });
		mat4 transform_controller = mat4(ovr->flip * right.transform) * mRot;

		vec3 origin = vec3(transform_controller * vec4{0.0f, 0.0f, 0.0f, 1.0f});
		vec3 p_forward = vec3(transform_controller * vec4{0.0f, 1.0f, 0.0f, 1.0f});
		vec3 p_right = vec3(transform_controller * vec4{1.0f, 0.0f, 0.0f, 1.0f});
		vec3 forward = -normalize(p_forward - origin);
		vec3 vright = -normalize(p_right - origin);

		vec3 delta = forward * float(GLRenderer::timeSinceLastFrame) * right.state.rAxis[0].y;
		delta = delta + vright * float(GLRenderer::timeSinceLastFrame) * right.state.rAxis[0].x;

		mat4 transform = translate(delta);

		scene.world->transform = transform * scene.world->transform;
	}

	// ============================================================
	// LEFT MENU: TOGGLE RING
	// ============================================================
	if (left.valid && left.isMenuPressed() && !left.wasMenuPressed()) {
		settings.showRing = !settings.showRing;
		//println("[VR] Ring: {}", settings.showRing ? "ON" : "OFF");
	}

	if (teleport_mode == TeleportMode::INDICATOR_TELEPORT || teleport_mode == TeleportMode::PLACE_INDICATOR) {
		SNSplats* indicatorEnvironment = editor->getTeleportEnvironmentLayer();
		for (size_t i = 0; i < teleport_indicators.size(); ++i) {
			const TeleportIndicator& indicator = teleport_indicators[i];

			if (!indicatorEnvironment) {
				continue;
			}

			const mat4 indicatorTransform =
				scene.world->transform *
				indicatorEnvironment->transform;

			const vec3 position = vec3(
				indicatorTransform *
				vec4(indicator.localPosition, 1.0f)
			);

			/*const vec3 position = vec3(
				scene.world->transform *
				vec4(indicator.localPosition, 1.0f)
			);*/


			const float radius = 0.12f;
			const float height = 0.25f;

			/*const vec3 px = vec3(radius, 0.0f, 0.0f);
			const vec3 py = vec3(0.0f, radius, 0.0f);
			const vec3 pz = vec3(0.0f, 0.0f, height);*/
			const mat3 basisRaw = mat3(indicatorTransform);

			const mat3 currentBasis(
				glm::normalize(vec3(basisRaw[0])),
				glm::normalize(vec3(basisRaw[1])),
				glm::normalize(vec3(basisRaw[2]))
			);

			const mat3 deltaRotation = currentBasis * glm::inverse(indicator.basisAtCreation);

			const vec3 px = deltaRotation * vec3(radius, 0.0f, 0.0f);
			const vec3 py = deltaRotation * vec3(0.0f, radius, 0.0f);
			const vec3 pz = deltaRotation * vec3(0.0f, 0.0f, height);

			const uint32_t color =
				static_cast<int>(i) == hovered_teleport_indicator
				? 0xff0000ff
				: 0xffffffff;

			drawLine(position + px, position + py, color);
			drawLine(position + py, position - px, color);
			drawLine(position - px, position - py, color);
			drawLine(position - py, position + px, color);

			drawLine(position, position + pz, color);
		}
	}

	#endif
}