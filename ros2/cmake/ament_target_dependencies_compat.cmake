# ROS 2 Lyrical removed ament_target_dependencies(). Keep local package
# CMakeLists readable while linking through exported imported targets. A few
# third-party ament packages still export only legacy include/library variables,
# so retain a narrow fallback for those packages.
if(NOT COMMAND ament_target_dependencies)
  function(_mowgli_append_dependency target visibility)
    set(link_items ${ARGN})

    if(visibility STREQUAL "INTERFACE")
      set_property(TARGET "${target}" APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES ${link_items})
    else()
      set_property(TARGET "${target}" APPEND PROPERTY
        LINK_LIBRARIES ${link_items})
      if(visibility STREQUAL "PUBLIC")
        set_property(TARGET "${target}" APPEND PROPERTY
          INTERFACE_LINK_LIBRARIES ${link_items})
      endif()
    endif()
  endfunction()

  function(ament_target_dependencies target)
    if(NOT TARGET "${target}")
      message(FATAL_ERROR
        "ament_target_dependencies compatibility: unknown target '${target}'")
    endif()

    set(visibility PUBLIC)
    set(system_includes FALSE)

    foreach(package IN LISTS ARGN)
      if(package STREQUAL "PUBLIC" OR
         package STREQUAL "PRIVATE" OR
         package STREQUAL "INTERFACE")
        set(visibility "${package}")
        continue()
      elseif(package STREQUAL "SYSTEM")
        set(system_includes TRUE)
        continue()
      endif()

      set(targets_variable "${package}_TARGETS")
      set(libraries_variable "${package}_LIBRARIES")
      set(library_variable "${package}_LIBRARY")
      set(include_dirs_variable "${package}_INCLUDE_DIRS")
      set(library_dirs_variable "${package}_LIBRARY_DIRS")
      set(definitions_variable "${package}_DEFINITIONS")
      set(dependency_targets)

      if(DEFINED ${targets_variable} AND
         NOT "${${targets_variable}}" STREQUAL "")
        set(dependency_targets ${${targets_variable}})
      elseif(TARGET "${package}::${package}")
        set(dependency_targets "${package}::${package}")
      elseif(package STREQUAL "behaviortree_cpp" AND
             TARGET BT::behaviortree_cpp)
        set(dependency_targets BT::behaviortree_cpp)
      elseif(DEFINED ${libraries_variable} AND
             NOT "${${libraries_variable}}" STREQUAL "")
        set(dependency_targets ${${libraries_variable}})
      elseif(DEFINED ${library_variable} AND
             NOT "${${library_variable}}" STREQUAL "")
        set(dependency_targets ${${library_variable}})
      else()
        message(FATAL_ERROR
          "ament_target_dependencies compatibility: package '${package}' "
          "does not export a supported target or library variable")
      endif()

      _mowgli_append_dependency(
        "${target}" "${visibility}" ${dependency_targets})

      if(DEFINED ${include_dirs_variable} AND
         NOT "${${include_dirs_variable}}" STREQUAL "")
        if(system_includes)
          target_include_directories(
            "${target}" SYSTEM ${visibility} ${${include_dirs_variable}})
        else()
          target_include_directories(
            "${target}" ${visibility} ${${include_dirs_variable}})
        endif()
      endif()

      if(DEFINED ${library_dirs_variable} AND
         NOT "${${library_dirs_variable}}" STREQUAL "")
        target_link_directories(
          "${target}" ${visibility} ${${library_dirs_variable}})
      endif()

      if(DEFINED ${definitions_variable} AND
         NOT "${${definitions_variable}}" STREQUAL "")
        target_compile_definitions(
          "${target}" ${visibility} ${${definitions_variable}})
      endif()
    endforeach()
  endfunction()
endif()
