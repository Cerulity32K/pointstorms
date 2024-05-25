#pragma once

#include "gl-cpp.hpp"
#include <string>

namespace kali {
	// vao
	template <typename T> vao<T>::vao() {
		glGenVertexArrays(1, &handle);
	}
	template <typename T> void vao<T>::bind() {
		glBindVertexArray(handle);
	}
	template <typename T> void vao<T>::reset() {
		for (size_t i = 0; i < index; i++) {
			glDisableVertexAttribArray(i);
		}
		index = 0;
	}
	template <typename T> void vao<T>::add_attribute(size_t size, size_t offset, GLenum type, GLboolean normalized) {
		bind();
		glVertexAttribPointer(index, size, type, normalized, sizeof(T), (const void*)offset);
		glEnableVertexAttribArray(index);
		index++;
	}
	template <typename T> vao<T>::vao(vao&& rhs) : handle(rhs.handle), index(rhs.index) {
		rhs.handle = 0;
		rhs.index = 0;
	}
	template <typename T> vao<T>& vao<T>::operator=(vao&& rhs) {
		handle = rhs.handle;
		index = rhs.index;
		rhs.handle = 0;
		rhs.index = 0;
		return *this;
	}
	template <typename T> vao<T>::~vao() {
		if (handle) glDeleteVertexArrays(1, &handle);
	}
	
	// buffer
	template <typename T> buffer<T>::buffer() {
		glGenBuffers(1, &binding);
	}
	template <typename T> buffer<T>::buffer(buffer&& rhs) : binding(rhs.binding) {
		rhs.binding = 0;
	}
	template <typename T> buffer<T>& buffer<T>::operator=(buffer&& rhs) {
		binding = rhs.binding;
		rhs.binding = 0;
	}
	template <typename T> buffer<T>::~buffer() {
		if (binding) glDeleteBuffers(1, &binding);
	}

	template <typename T> void buffer<T>::bind(GLenum target) {
		glBindBuffer(target, binding);
	}
	template <typename T> void buffer<T>::set(GLenum target, T* data, size_t count, GLenum usage_hint) {
		bind(target);
		glBufferData(target, count * sizeof(T), data, usage_hint);
	}
}
