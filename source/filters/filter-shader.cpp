// AUTOGENERATED COPYRIGHT HEADER START
// Copyright (C) 2019-2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
// Copyright (C) 2021 coolsoft.rf <coolsoft.rf@gmail.com>
// AUTOGENERATED COPYRIGHT HEADER END

#include "filter-shader.hpp"
#include "strings.hpp"
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"

#include "warning-disable.hpp"
#include <stdexcept>
#include "warning-enable.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<filter::shader> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#define ST_I18N "Filter.Shader"

using namespace streamfx::filter::shader;

static constexpr std::string_view HELP_URL = "https://github.com/Xaymar/obs-StreamFX/wiki/Source-Filter-Transition-Shader";

shader_instance::shader_instance(obs_data_t* data, obs_source_t* self) : obs::source_instance(data, self)
{
	_fx = std::make_shared<streamfx::gfx::shader::shader>(self, streamfx::gfx::shader::shader_mode::Filter);
	_rt = std::make_shared<streamfx::obs::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);

	update(data);
}

shader_instance::~shader_instance() {}

uint32_t shader_instance::get_width()
{
	return _fx->width();
}

uint32_t shader_instance::get_height()
{
	return _fx->height();
}

void shader_instance::properties(obs_properties_t* props)
{
	_fx->properties(props);
}

void shader_instance::load(obs_data_t* data)
{
	update(data);
}

void shader_instance::migrate(obs_data_t* data, uint64_t version) {}

void shader_instance::update(obs_data_t* data)
{
	_fx->update(data);
}

void shader_instance::video_tick(float_t sec_since_last)
{
	if (_fx->tick(sec_since_last)) {
		obs_data_t* data = obs_source_get_settings(_self);
		_fx->update(data);
		obs_data_release(data);
	}

	if (obs_source_t* tgt = obs_filter_get_target(_self); tgt != nullptr) {
		_fx->set_size(obs_source_get_base_width(tgt), obs_source_get_base_height(tgt));
	} else if (obs_source* src = obs_filter_get_parent(_self); src != nullptr) {
		_fx->set_size(obs_source_get_base_width(src), obs_source_get_base_height(src));
	}
}

void shader_instance::video_render(gs_effect_t* effect)
{
	try {
		if (!_fx || !_fx->base_width() || !_fx->base_height()) {
			throw std::runtime_error("No effect, or invalid base size.");
		}

#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
		streamfx::obs::gs::debug_marker gdmp{streamfx::obs::gs::debug_color_source, "Shader Filter '%s' on '%s'", obs_source_get_name(_self), obs_source_get_name(obs_filter_get_parent(_self))};
#endif

		{
#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
			streamfx::obs::gs::debug_marker gdm{streamfx::obs::gs::debug_color_source, "Cache"};
#endif

			auto op = _rt->render(_fx->base_width(), _fx->base_height());

			gs_ortho(0, 1, 0, 1, -1, 1);

			/// Render original source
			if (obs_source_process_filter_begin(_self, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
				gs_blend_state_push();
				gs_reset_blend_state();
				gs_blend_function_separate(GS_BLEND_ONE, GS_BLEND_ZERO, GS_BLEND_SRCALPHA, GS_BLEND_ZERO);
				gs_enable_blending(false);
				gs_enable_depth_test(false);
				gs_enable_stencil_test(false);
				gs_enable_stencil_write(false);
				gs_enable_color(true, true, true, true);
				gs_set_cull_mode(GS_NEITHER);

				obs_source_process_filter_end(_self, obs_get_base_effect(OBS_EFFECT_DEFAULT), 1, 1);

				gs_blend_state_pop();
			} else {
				throw std::runtime_error("Failed to render previous source.");
			}
		}

		{
#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
			streamfx::obs::gs::debug_marker gdm{streamfx::obs::gs::debug_color_render, "Render"};
#endif

			_fx->prepare_render();
			_fx->set_input_a(_rt->get_texture());
			_fx->render(effect);
		}
	} catch (const std::exception& ex) {
		obs_source_skip_video_filter(_self);
		throw ex;
	}
}

void streamfx::filter::shader::shader_instance::show()
{
	_fx->set_visible(true);
}

void streamfx::filter::shader::shader_instance::hide()
{
	_fx->set_visible(false);
}

void streamfx::filter::shader::shader_instance::activate()
{
	_fx->set_active(true);
}

void streamfx::filter::shader::shader_instance::deactivate()
{
	_fx->set_active(false);
}

shader_factory::shader_factory()
{
	_info.id           = S_PREFIX "filter-shader";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	support_activity_tracking(true);
	support_visibility_tracking(true);
	finish_setup();
	register_proxy("obs-stream-effects-filter-shader");
}

shader_factory::~shader_factory() {}

const char* shader_factory::get_name()
{
	return D_TRANSLATE(ST_I18N);
}

void shader_factory::get_defaults2(obs_data_t* data)
{
	streamfx::gfx::shader::shader::defaults(data);
}

obs_properties_t* shader_factory::get_properties2(shader::shader_instance* data)
{
	auto pr = obs_properties_create();
	obs_properties_set_param(pr, data, nullptr);

#ifdef ENABLE_FRONTEND
	{
		auto p = obs_properties_add_button2(pr, S_MANUAL_OPEN, D_TRANSLATE(S_MANUAL_OPEN), streamfx::filter::shader::shader_factory::on_manual_open, nullptr);
	}
#endif

	if (data) {
		reinterpret_cast<shader_instance*>(data)->properties(pr);
	}

	return pr;
}

#ifdef ENABLE_FRONTEND
bool shader_factory::on_manual_open(obs_properties_t* props, obs_property_t* property, void* data)
{
	try {
		streamfx::open_url(HELP_URL);
		return false;
	} catch (const std::exception& ex) {
		D_LOG_ERROR("Failed to open manual due to error: %s", ex.what());
		return false;
	} catch (...) {
		D_LOG_ERROR("Failed to open manual due to unknown error.", "");
		return false;
	}
}
#endif

std::shared_ptr<shader_factory> _filter_shader_factory_instance = nullptr;

void streamfx::filter::shader::shader_factory::initialize()
{
	try {
		if (!_filter_shader_factory_instance)
			_filter_shader_factory_instance = std::make_shared<shader_factory>();
	} catch (const std::exception& ex) {
		D_LOG_ERROR("Failed to initialize due to error: %s", ex.what());
	} catch (...) {
		D_LOG_ERROR("Failed to initialize due to unknown error.", "");
	}
}

void streamfx::filter::shader::shader_factory::finalize()
{
	_filter_shader_factory_instance.reset();
}

std::shared_ptr<shader_factory> streamfx::filter::shader::shader_factory::get()
{
	return _filter_shader_factory_instance;
}
