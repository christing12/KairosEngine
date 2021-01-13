#pragma once

#define KRS_CLASS_DEFAULT( className ) \
className(void) = default; \
~className(void) = default; \
className(const className& rhs) = default; \
className& operator=(const className& rhs) = default; \
className(className&& other) = default; \
className& operator=(className&& other) = default;

#define KRS_CLASS_NON_COPYABLE( className ) \
className(void) = default; \
~className(void) = default; \
className(const className& rhs) = delete; \
className& operator=(const className& rhs) = delete; \
className(className&& other) = default; \
className& operator=(className&& other) = default;

#define KRS_CLASS_NON_MOVABLE( className ) \
className(void) = default; \
~className(void) = default; \
className(const className& rhs) = default; \
className& operator=(const className& rhs) = default; \
className(className&& other) = delete; \
className& operator=(className&& other) = delete;

#define KRS_CLASS_NON_COPYABLE_AND_MOVABLE( className ) \
className(void) = default; \
~className(void) = default; \
className(const className& rhs) = delete; \
className& operator=(const className& rhs) = delete; \
className(className&& other) = delete; \
className& operator=(className&& other) = delete;



// -------------------- INTERFACES -----------------//


#define KRS_INTERFACE_DEFAULT( className ) \
className(void) = default; \
virtual ~className(void) = default; \
className(const className& rhs) = default; \
className& operator=(const className& rhs) = default; \
className(className&& other) = default; \
className& operator=(className&& other) = default;

#define KRS_INTERFACE_NON_COPYABLE( className ) \
className(void) = default; \
virtual ~className(void) = default; \
className(const className& rhs) = delete; \
className& operator=(const className& rhs) = delete; \
className(className&& other) = default; \
className& operator=(className&& other) = default;

#define KRS_INTERFACE_NON_MOVABLE( className ) \
className(void) = default; \
virtual ~className(void) = default; \
className(const className& rhs) = default; \
className& operator=(const className& rhs) = default; \
className(className&& other) = delete; \
className& operator=(className&& other) = delete;

#define KRS_INTERFACE_NON_COPYABLE_AND_NON_MOVABLE( className ) \
className(void) = default; \
virtual ~className(void) = default; \
className(const className& rhs) = delete; \
className& operator=(const className& rhs) = delete; \
className(className&& other) = delete; \
className& operator=(className&& other) = delete;



/* ------------------ INHERIT DEFINITIONS ------------------- */

struct NonCopyable {
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	auto operator=(const NonCopyable&) = delete;
};
