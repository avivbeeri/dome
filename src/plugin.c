internal DOME_Result
PLUGIN_nullHook(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

internal void
PLUGIN_COLLECTION_initRecord(PLUGIN_COLLECTION* plugins, size_t index) {
  plugins->active[index] = false;
  plugins->objectHandle[index] = NULL;
  plugins->name[index] = NULL;
  plugins->preUpdateHook[index] = PLUGIN_nullHook;
}

internal void
PLUGIN_COLLECTION_init(ENGINE* engine) {
  // TODO: handle allocation failures
  PLUGIN_COLLECTION plugins = engine->plugins;
  plugins.max = 1;
  plugins.count = 0;
  assert(plugins.count <= plugins.max);
  plugins.active = calloc(plugins.max, sizeof(bool));
  plugins.name = calloc(plugins.max, sizeof(char*));
  plugins.objectHandle = calloc(plugins.max, sizeof(void*));
  plugins.preUpdateHook = calloc(plugins.max, sizeof(DOME_Plugin_Hook));
  for (size_t i = 0; i < plugins.max; i++) {
    PLUGIN_COLLECTION_initRecord(&plugins, i);
  }
  engine->plugins = plugins;
}

internal void
PLUGIN_COLLECTION_free(ENGINE* engine) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  for (size_t i = 0; i < plugins.count; i++) {
    // TODO Call the unload here
    DOME_Plugin_Hook shutdownHook;
    shutdownHook = (DOME_Plugin_Hook)SDL_LoadFunction(plugins.objectHandle[i], "DOME_hookOnShutdown");
    if (shutdownHook != NULL) {
      shutdownHook(engine);
      // TODO: Log failure
    }
    plugins.active[i] = false;

    free(plugins.name[i]);
    plugins.name[i] = NULL;

    SDL_UnloadObject(plugins.objectHandle[i]);
    plugins.objectHandle[i] = NULL;

    plugins.preUpdateHook[i] = NULL;
  }

  plugins.max = 0;
  plugins.count = 0;

  free(plugins.active);
  plugins.active = NULL;

  free(plugins.name);
  plugins.name = NULL;

  free(plugins.objectHandle);
  plugins.objectHandle = NULL;

  engine->plugins = plugins;
}

internal DOME_Result
PLUGIN_COLLECTION_runHook(ENGINE* engine, DOME_PLUGIN_HOOK hook) {
  PLUGIN_COLLECTION plugins = engine->plugins;
  size_t failures = 0;

  for (size_t i = 0; i < plugins.count; i++) {
    assert(plugins.active[i]);
    DOME_Result result = DOME_RESULT_SUCCESS;
    switch (hook) {
      case DOME_PLUGIN_HOOK_PRE_UPDATE:
        { result = plugins.preUpdateHook[i](engine); } break;
      default: break;
    }
    if (result != DOME_RESULT_SUCCESS) {
      failures++;
      // Do something about errors?
    }
  }

  return DOME_RESULT_SUCCESS;
}

internal DOME_Result
PLUGIN_COLLECTION_add(ENGINE* engine, const char* name) {
  void* handle = SDL_LoadObject(name);
  if (handle == NULL) {
    ENGINE_printLog(engine, "%s\n", SDL_GetError());
    return DOME_RESULT_FAILURE;
  }

  PLUGIN_COLLECTION plugins = engine->plugins;
  size_t next = plugins.count;
  printf("Next is %zu, max is %zu\n", next, plugins.max);

  if (next >= plugins.max) {
    plugins.max *= 2;
    printf("Increasing max to %zu\n", plugins.max);

    plugins.active = realloc(plugins.active, sizeof(bool) * plugins.max);
    plugins.name = realloc(plugins.name, sizeof(char*) * plugins.max);
    plugins.objectHandle = realloc(plugins.objectHandle, sizeof(void*) * plugins.max);
    plugins.preUpdateHook = realloc(plugins.preUpdateHook, sizeof(void*) * plugins.max);
    for (size_t i = next + 1; i < plugins.max; i++) {
      PLUGIN_COLLECTION_initRecord(&plugins, i);
    }
    // TODO handle allocation failure
  }
  plugins.active[next] = true;
  plugins.name[next] = strdup(name);
  plugins.objectHandle[next] = handle;
  plugins.count++;

  // Acquire hook function pointers
  DOME_Plugin_Hook hook;
  hook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "DOME_hookOnPreUpdate");
  if (hook != NULL) {
    plugins.preUpdateHook[next] = hook;
  }

  engine->plugins = plugins;

  DOME_Plugin_Hook initHook;
  initHook = (DOME_Plugin_Hook)SDL_LoadFunction(handle, "DOME_hookOnInit");
  if (initHook != NULL) {
    return initHook(engine);
  }

  return DOME_RESULT_SUCCESS;
}

external DOME_Result
DOME_registerModule(DOME_Context ctx, const char* name, const char* source) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  MAP_addModule(moduleMap, name, source);

  return DOME_RESULT_SUCCESS;
}

external DOME_Result
DOME_registerBindFn(DOME_Context ctx, const char* moduleName, DOME_BindClassFn fn) {
  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  return MAP_bindForeignClass(moduleMap, moduleName, fn);
}

external DOME_Result
DOME_registerFnImpl(DOME_Context ctx, const char* moduleName, const char* signature, DOME_ForeignFn method) {

  ENGINE* engine = (ENGINE*)ctx;
  MAP* moduleMap = &(engine->moduleMap);
  MAP_addFunction(moduleMap, moduleName, signature, (WrenForeignMethodFn)method);

  return DOME_RESULT_SUCCESS;
}

external DOME_Context
DOME_getContext(void* vm) {
  return wrenGetUserData(vm);
}

external bool
DOME_setSlot(void* vm, size_t slot, DOME_SLOT_TYPE type, ...) {
  va_list argp;
  va_start(argp, type);

  DOME_SLOT_VALUE value;
  switch (type) {
    case DOME_SLOT_TYPE_NULL:
      wrenSetSlotNull(vm, slot); break;
    case DOME_SLOT_TYPE_BOOL:
      value.as.boolean = va_arg(argp, int);
      wrenSetSlotBool(vm, slot, value.as.boolean); break;
    case DOME_SLOT_TYPE_NUMBER:
      value.as.number = va_arg(argp, double);
      wrenSetSlotDouble(vm, slot, value.as.number);
      break;
    case DOME_SLOT_TYPE_STRING:
      value.as.text = va_arg(argp, char*);
      wrenSetSlotString(vm, slot, value.as.text);
      break;
    case DOME_SLOT_TYPE_BYTES:
      value.as.bytes.data = va_arg(argp, char*);
      value.as.bytes.len = va_arg(argp, size_t);
      wrenSetSlotBytes(vm, slot, value.as.bytes.data, value.as.bytes.len);
      break;
    default:
      VM_ABORT(vm, "Unhandled plugin return type"); break;
      va_end(argp);
      return false;
  }
  va_end(argp);
  return true;
}

DOME_EXPORTED DOME_SLOT_VALUE DOME_getSlot(void* vm, size_t slot, DOME_SLOT_TYPE type) {
  DOME_SLOT_VALUE result;
  result.as.boolean = false;
  switch (type) {
    case DOME_SLOT_TYPE_BOOL:
      if (wrenGetSlotType(vm, slot) != WREN_TYPE_BOOL) {
        VM_ABORT(vm, "Incorrect plugin argument type");
        break;
      }
      result.as.boolean = wrenGetSlotBool(vm, slot);
      break;
    case DOME_SLOT_TYPE_NUMBER:
      if (wrenGetSlotType(vm, slot) != WREN_TYPE_NUM) {
        VM_ABORT(vm, "Incorrect plugin argument type");
        break;
      }
      result.as.number = wrenGetSlotDouble(vm, slot);
      break;
    case DOME_SLOT_TYPE_STRING:
      if (wrenGetSlotType(vm, slot) != WREN_TYPE_STRING) {
        VM_ABORT(vm, "Incorrect plugin argument type");
        break;
      }
      result.as.text = wrenGetSlotString(vm, slot);
      break;
    case DOME_SLOT_TYPE_BYTES:
      if (wrenGetSlotType(vm, slot) != WREN_TYPE_STRING) {
        VM_ABORT(vm, "Incorrect plugin argument type");
        break;
      }
      result.as.bytes.data = wrenGetSlotBytes(vm, slot, (int*)&result.as.bytes.len);
      break;
    default:
      VM_ABORT(vm, "Unhandled plugin argument type"); break;
  }
  return result;
}