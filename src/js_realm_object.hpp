////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace realm {
namespace js {
    template<typename> struct RealmObjectClass;
}
}

#include "object_accessor.hpp"
#include "object_store.hpp"

#include "js_class.hpp"
#include "js_types.hpp"
#include "js_util.hpp"
#include "js_realm.hpp"
#include "js_schema.hpp"

namespace realm {
namespace js {

template<typename> class NativeAccessor;

template<typename T>
class RealmObject : public realm::Object {
public:
    RealmObject(RealmObject const& obj) : realm::Object(obj) {};
    RealmObject(realm::Object const& obj) : realm::Object(obj) {};
    RealmObject(RealmObject&&) = default;
    RealmObject& operator=(RealmObject&&) = default;
    RealmObject& operator=(RealmObject const&) = default;

    std::vector<std::pair<Protected<typename T::Function>, NotificationToken>> m_notification_tokens;
};

template<typename T>
struct RealmObjectClass : ClassDefinition<T, realm::js::RealmObject<T>> {
    using ContextType = typename T::Context;
    using FunctionType = typename T::Function;
    using ObjectType = typename T::Object;
    using ValueType = typename T::Value;
    using String = js::String<T>;
    using Value = js::Value<T>;
    using Object = js::Object<T>;
    using Function = js::Function<T>;
    using ReturnValue = js::ReturnValue<T>;
    using Arguments = js::Arguments<T>;

    static ObjectType create_instance(ContextType, realm::js::RealmObject<T>);

    static void get_property(ContextType, ObjectType, const String &, ReturnValue &);
    static bool set_property(ContextType, ObjectType, const String &, ValueType);
    static std::vector<String> get_property_names(ContextType, ObjectType);

    static void is_valid(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void get_object_schema(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void linking_objects(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void linking_objects_count(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void get_object_id(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void is_same_object(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void set_link(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void add_listener(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void remove_listener(ContextType, ObjectType, Arguments &, ReturnValue &);
    static void remove_all_listeners(ContextType, ObjectType, Arguments &, ReturnValue &);

    static void get_realm(ContextType, ObjectType, ReturnValue &);

    const std::string name = "RealmObject";

    const StringPropertyType<T> string_accessor = {
        wrap<get_property>,
        wrap<set_property>,
        wrap<get_property_names>,
    };

    MethodMap<T> const methods = {
        {"isValid", wrap<is_valid>},
        {"objectSchema", wrap<get_object_schema>},
        {"linkingObjects", wrap<linking_objects>},
        {"linkingObjectsCount", wrap<linking_objects_count>},
        {"_objectId", wrap<get_object_id>},
        {"_isSameObject", wrap<is_same_object>},
        {"_setLink", wrap<set_link>},
        {"addListener", wrap<add_listener>},
        {"removeListener", wrap<remove_listener>},
        {"removeAllListeners", wrap<remove_all_listeners>},
    };

    PropertyMap<T> const properties = {
        {"_realm", {wrap<get_realm>, nullptr}},
    };
};

template<typename T>
void RealmObjectClass<T>::is_valid(ContextType, ObjectType this_object, Arguments &, ReturnValue &return_value) {
    return_value.set(get_internal<T, RealmObjectClass<T>>(this_object)->is_valid());
}

template<typename T>
void RealmObjectClass<T>::get_object_schema(ContextType ctx, ObjectType this_object, Arguments &, ReturnValue &return_value) {
    auto object = get_internal<T, RealmObjectClass<T>>(this_object);
    return_value.set(Schema<T>::object_for_object_schema(ctx, object->get_object_schema()));
}

template<typename T>
typename T::Object RealmObjectClass<T>::create_instance(ContextType ctx, realm::js::RealmObject<T> realm_object) {
    static String prototype_string = "prototype";

    auto delegate = get_delegate<T>(realm_object.realm().get());
    auto schema = realm_object.get_object_schema();
    auto name = schema.name;

    auto internal = new realm::js::RealmObject<T>(std::move(realm_object));
    try {
        if (!delegate || !delegate->m_constructors.count(name)) {
            FunctionType constructor;
            auto object = create_instance_by_schema<T, RealmObjectClass<T>>(ctx, constructor, schema, internal);
            return object;
        }

        FunctionType constructor = delegate->m_constructors.at(name);
        auto object = create_instance_by_schema<T, RealmObjectClass<T>>(ctx, constructor, schema, internal);
        return object;
    }
    catch (const std::exception& e) {
        delete internal;
        throw;
    }
}

template<typename T>
void RealmObjectClass<T>::get_property(ContextType ctx, ObjectType object, const String &property_name, ReturnValue &return_value) {
    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    std::string prop_name = property_name;
    const Property* prop = realm_object->get_object_schema().property_for_public_name(prop_name);
    if (prop) {
        NativeAccessor<T> accessor(ctx, realm_object->realm(), realm_object->get_object_schema());
        auto result = realm_object->template get_property_value<ValueType>(accessor, *prop);
        return_value.set(result);
    }
}

template<typename T>
bool RealmObjectClass<T>::set_property(ContextType ctx, ObjectType object, const String &property_name, ValueType value) {
    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    std::string prop_name = property_name;
    const Property* prop = realm_object->get_object_schema().property_for_public_name(prop_name);
    if (!prop) {
        return false;
    }

    NativeAccessor<T> accessor(ctx, realm_object->realm(), realm_object->get_object_schema());
    if (!Value::is_valid_for_property(ctx, value, *prop)) {
        throw TypeErrorException(accessor, realm_object->get_object_schema().name, *prop, value);
    }

    realm_object->set_property_value(accessor, prop->name, value, realm::CreatePolicy::UpdateAll);
    return true;
}

template<typename T>
void RealmObjectClass<T>::set_link(ContextType ctx, ObjectType object, Arguments &args, ReturnValue& return_value) {
    args.validate_count(2);

    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    realm_object->realm()->verify_in_write();

    NativeAccessor<T> accessor(ctx, realm_object->realm(), realm_object->get_object_schema());
    std::string property_name = Value::validated_to_string(ctx, args[0], "propertyName");
    const Property* prop = realm_object->get_object_schema().property_for_name(property_name);
    if (!prop) {
        throw std::invalid_argument(util::format("No such property: %1", property_name));
    }
    if (prop->type != realm::PropertyType::Object) {
        throw TypeErrorException(accessor, realm_object->get_object_schema().name, *prop, args[1]);
    }
    auto& linked_schema = *realm_object->realm()->schema().find(prop->object_type);
    auto linked_pk = linked_schema.primary_key_property();
    if (!linked_pk) {
        throw std::invalid_argument("Linked object type must have a primary key.");
    }

    auto table = realm_object->row().get_table();
    auto linked_table = table->get_link_target(prop->table_column);

    size_t row_ndx = realm::not_found;
    if (linked_pk->type == realm::PropertyType::String) {
        row_ndx = linked_table->find_first(linked_pk->table_column,
                                           accessor.template unbox<StringData>(args[1]));
    }
    else if (is_nullable(linked_pk->type)) {
        row_ndx = linked_table->find_first(linked_pk->table_column,
                                           accessor.template unbox<util::Optional<int64_t>>(args[1]));
    }
    else {
        row_ndx = linked_table->find_first(linked_pk->table_column,
                                           accessor.template unbox<int64_t>(args[1]));
    }

    if (row_ndx == realm::not_found) {
        realm_object->row().nullify_link(prop->table_column);
    }
    else {
        realm_object->row().set_link(prop->table_column, row_ndx);
    }
}

template<typename T>
void RealmObjectClass<T>::get_realm(ContextType ctx, ObjectType object, ReturnValue& return_value) {
    return_value.set_undefined();
    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    if (realm_object) {
        ObjectType realm_obj = create_object<T, RealmClass<T>>(ctx, new SharedRealm(realm_object->realm()));
        return_value.set(realm_obj);
    }
}

template<typename T>
std::vector<String<T>> RealmObjectClass<T>::get_property_names(ContextType ctx, ObjectType object) {
    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    auto &object_schema = realm_object->get_object_schema();

    std::vector<String> names;
    names.reserve(object_schema.persisted_properties.size() + object_schema.computed_properties.size());

    for (auto &prop : object_schema.persisted_properties) {
        names.push_back(!prop.public_name.empty() ? prop.public_name : prop.name);
    }
    for (auto &prop : object_schema.computed_properties) {
        names.push_back(!prop.public_name.empty() ? prop.public_name : prop.name);
    }

    return names;
}

template<typename T>
void RealmObjectClass<T>::get_object_id(ContextType ctx, ObjectType object, Arguments &args, ReturnValue& return_value) {
    args.validate_maximum(0);

#if REALM_ENABLE_SYNC
    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    const Group& group = realm_object->realm()->read_group();
    if (!sync::has_object_ids(group))
        throw std::logic_error("_objectId() can only be used with objects from synced Realms.");

    const Row& row = realm_object->row();
    auto object_id = sync::object_id_for_row(group, *row.get_table(), row.get_index());
    return_value.set(object_id.to_string());
#else
    throw std::logic_error("_objectId() can only be used with objects from synced Realms.");
#endif
}

template<typename T>
void RealmObjectClass<T>::is_same_object(ContextType ctx, ObjectType object, Arguments &args, ReturnValue& return_value) {
    args.validate_count(1);

    ObjectType otherObject = Value::validated_to_object(ctx, args[0]);
    if (!Object::template is_instance<RealmObjectClass<T>>(ctx, otherObject)) {
        return_value.set(false);
        return;
    }

    auto self = get_internal<T, RealmObjectClass<T>>(object);
    auto other = get_internal<T, RealmObjectClass<T>>(otherObject);

    if (!self->realm() || self->realm() != other->realm()) {
        return_value.set(false);
        return;
    }

    if (!self->is_valid() || !other->is_valid()) {
        return_value.set(false);
        return;
    }

    return_value.set(self->row().get_table() == other->row().get_table()
                     && self->row().get_index() == other->row().get_index());
}

template<typename T>
void RealmObjectClass<T>::linking_objects_count(ContextType, ObjectType object, Arguments &, ReturnValue &return_value) {
    auto realm_object = get_internal<T, RealmObjectClass<T>>(object);
    const Row& row = realm_object->row();

    return_value.set((uint32_t)row.get_backlink_count());
}


template<typename T>
void RealmObjectClass<T>::add_listener(ContextType ctx, ObjectType this_object, Arguments &args, ReturnValue& return_value) {
    args.validate_maximum(1);

    auto realm_object = get_internal<T, RealmObjectClass<T>>(this_object);

    auto callback = Value::validated_to_function(ctx, args[0]);
    Protected<FunctionType> protected_callback(ctx, callback);
    Protected<ObjectType> protected_this(ctx, this_object);
    Protected<typename T::GlobalContext> protected_ctx(Context<T>::get_global_context(ctx));

    auto token = realm_object->add_notification_callback([=](CollectionChangeSet const& change_set, std::exception_ptr exception) {
            HANDLESCOPE(protected_ctx)

            bool deleted = false;
            std::vector<ValueType> scratch;

            if (!change_set.deletions.empty()) {
                deleted = true;
            }
            else {
                auto table = realm_object->row().get_table();
                for (size_t i = 0; i < change_set.columns.size(); ++i) {
                    if (change_set.columns[i].empty()) {
                        continue;
                    }
                    scratch.push_back(Value::from_string(protected_ctx, std::string(table->get_column_name(i))));
                }
            }

            ObjectType object = Object::create_empty(protected_ctx);
            Object::set_property(protected_ctx, object, "deleted", Value::from_boolean(protected_ctx, deleted));
            Object::set_property(protected_ctx, object, "changedProperties", Object::create_array(protected_ctx, scratch));

            ValueType arguments[] {
                static_cast<ObjectType>(protected_this),
                object
            };
            Function::callback(protected_ctx, protected_callback, protected_this, 2, arguments);
        });
    realm_object->m_notification_tokens.emplace_back(protected_callback, std::move(token));
}

template<typename T>
void RealmObjectClass<T>::remove_listener(ContextType ctx, ObjectType this_object, Arguments &args, ReturnValue &return_value) {
    args.validate_maximum(1);

    auto callback = Value::validated_to_function(ctx, args[0]);
    auto protected_function = Protected<FunctionType>(ctx, callback);

    auto realm_object = get_internal<T, RealmObjectClass<T>>(this_object);

    auto& tokens = realm_object->m_notification_tokens;
    auto compare = [&](auto&& token) {
        return typename Protected<FunctionType>::Comparator()(token.first, protected_function);
    };
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(), compare), tokens.end());
}

template<typename T>
void RealmObjectClass<T>::remove_all_listeners(ContextType ctx, ObjectType this_object, Arguments &args, ReturnValue &return_value) {
    args.validate_maximum(0);

    auto realm_object = get_internal<T, RealmObjectClass<T>>(this_object);
    realm_object->m_notification_tokens.clear();
}


} // js
} // realm

// move this all the way here because it needs to include "js_results.hpp" which in turn includes this file

#include "js_results.hpp"

template<typename T>
void realm::js::RealmObjectClass<T>::linking_objects(ContextType ctx, ObjectType this_object, Arguments &args, ReturnValue &return_value) {
    args.validate_count(2);

    std::string object_type = Value::validated_to_string(ctx, args[0], "objectType");
    std::string property_name = Value::validated_to_string(ctx, args[1], "property");

    auto object = get_internal<T, RealmObjectClass<T>>(this_object);

    auto target_object_schema = object->realm()->schema().find(object_type);
    if (target_object_schema == object->realm()->schema().end()) {
        throw std::logic_error(util::format("Could not find schema for type '%1'", object_type));
    }

    auto link_property = target_object_schema->property_for_name(property_name);
    if (!link_property) {
        throw std::logic_error(util::format("Type '%1' does not contain property '%2'", object_type, property_name));
    }

    if (link_property->object_type != object->get_object_schema().name) {
        throw std::logic_error(util::format("'%1.%2' is not a relationship to '%3'", object_type, property_name, object->get_object_schema().name));
    }

    realm::TableRef table = ObjectStore::table_for_object_type(object->realm()->read_group(), target_object_schema->name);
    auto row = object->row();
    auto tv = row.get_table()->get_backlink_view(row.get_index(), table.get(), link_property->table_column);

    return_value.set(ResultsClass<T>::create_instance(ctx, realm::Results(object->realm(), std::move(tv))));
}
