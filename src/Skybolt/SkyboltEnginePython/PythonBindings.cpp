/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/OrbitComponent.h>
#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ParentReferenceComponent.h>
#include <SkyboltSim/Components/ProceduralLifetimeComponent.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace skybolt;
using namespace skybolt::sim;

World* gWorld = nullptr;
static World* getWorld() { return gWorld; }
static void setWorld(World* world) { gWorld = world; }

EntityFactory* gEntityFactory = nullptr;
static EntityFactory* getEntityFactory() { return gEntityFactory; }
static void setEntityFactory(EntityFactory* factory) { gEntityFactory = factory; }

static void removeNamespaceQualifier(std::string& str)
{
	size_t p = str.find_last_of(":");
	if (p != std::string::npos)
	{
		str = str.substr(p + 1);
	}
}

static std::vector<ComponentPtr> getComponentsOfTypeName(Entity* entity, const std::string& typeName)
{
	std::vector<ComponentPtr> result;

	std::vector<ComponentPtr> components = entity->getComponents();
	for (const ComponentPtr& component : components)
	{
		std::string name(typeid(*component).name());
		py::detail::clean_type_id(name);
		removeNamespaceQualifier(name);
		if (name == typeName)
		{
			result.push_back(component);
		}
	}
	return result;
};

static ComponentPtr getFirstComponentOfTypeName(Entity* entity, const std::string& typeName)
{
	const auto v = getComponentsOfTypeName(entity, typeName);
	return v.empty() ? nullptr : v.front();
}

static double dotFunc(const Vector3& a, const Vector3& b)
{
	return glm::dot(a, b);
}

static Vector3 crossFunc(const Vector3& a, const Vector3& b)
{
	return glm::cross(a, b);
}

static Vector3 normalizeFunc(const Vector3& v)
{
	return glm::normalize(v);
}


PYBIND11_MODULE(skybolt, m) {
	py::class_<Vector3>(m, "Vector3")
		.def(py::init())
		.def(py::init<double, double, double>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= double())
		.def(double() * py::self);

	py::class_<Quaternion>(m, "Quaternion")
		.def(py::init())
		.def(py::init<double, double, double, double>())
		.def_readwrite("x", &Quaternion::x)
		.def_readwrite("y", &Quaternion::y)
		.def_readwrite("z", &Quaternion::z)
		.def_readwrite("w", &Quaternion::w);

	py::class_<LatLon>(m, "LatLon")
		.def(py::init())
		.def(py::init<double, double>())
		.def_readwrite("lat", &LatLon::lat)
		.def_readwrite("lon", &LatLon::lon);

	py::class_<LatLonAlt>(m, "LatLonAlt")
		.def(py::init())
		.def(py::init<double, double, double>())
		.def_readwrite("lat", &LatLonAlt::lat)
		.def_readwrite("lon", &LatLonAlt::lon)
		.def_readwrite("alt", &LatLonAlt::alt);

	py::class_<Orbit>(m, "Orbit")
		.def(py::init())
		.def_readwrite("semiMajorAxis", &Orbit::semiMajorAxis)
		.def_readwrite("eccentricity", &Orbit::eccentricity)
		.def_readwrite("inclination", &Orbit::inclination)
		.def_readwrite("rightAscension", &Orbit::rightAscension)
		.def_readwrite("argumentOfPeriapsis", &Orbit::argumentOfPeriapsis)
		.def_readwrite("trueAnomaly", &Orbit::trueAnomaly);
		

	py::class_<Position, std::shared_ptr<Position>>(m, "Position");

	py::class_<GeocentricPosition, std::shared_ptr<GeocentricPosition>, Position>(m, "GeocentricPosition")
		.def(py::init<Vector3>())
		.def_readwrite("position", &GeocentricPosition::position);

	py::class_<LatLonAltPosition, std::shared_ptr<LatLonAltPosition>, Position>(m, "LatLonAltPosition")
		.def(py::init<LatLonAlt>())
		.def_readwrite("position", &LatLonAltPosition::position);

	py::class_<Orientation, std::shared_ptr<Orientation>>(m, "Orientation");

	py::class_<GeocentricOrientation, std::shared_ptr<GeocentricOrientation>, Orientation>(m, "GeocentricOrientation")
		.def(py::init<Quaternion>())
		.def_readwrite("orientation", &GeocentricOrientation::orientation);

	py::class_<LtpNedOrientation, std::shared_ptr<LtpNedOrientation>, Orientation>(m, "LtpNedOrientation")
		.def(py::init<Quaternion>())
		.def_readwrite("orientation", &LtpNedOrientation::orientation);

	py::class_<Component, std::shared_ptr<Component>>(m, "Component");

	py::class_<OrbitComponent, std::shared_ptr<OrbitComponent>, Component>(m, "OrbitComponent")
		.def(py::init())
		.def_readwrite("orbit", &OrbitComponent::orbit);

	py::class_<MainRotorComponent, std::shared_ptr<MainRotorComponent>, Component>(m, "MainRotorComponent")
		.def("getPitchAngle", &MainRotorComponent::getPitchAngle)
		.def("getRotationAngle", &MainRotorComponent::getRotationAngle)
		.def("getTppOrientationRelBody", &MainRotorComponent::getTppOrientationRelBody)
		.def("setNormalizedRpm", &MainRotorComponent::setNormalizedRpm);

	py::class_<ParentReferenceComponent, std::shared_ptr<ParentReferenceComponent>, Component>(m, "ParentReferenceComponent")
		.def(py::init<sim::Entity*>())
		.def("getParent", &ParentReferenceComponent::getParent);

	py::class_<ProceduralLifetimeComponent, std::shared_ptr<ProceduralLifetimeComponent>, Component>(m, "ProceduralLifetimeComponent")
		.def(py::init());

	py::class_<Entity, std::shared_ptr<Entity>>(m, "Entity")
		.def("getName", [](Entity* entity) { return getName(*entity); })
		.def("getPosition", [](Entity* entity) {
			return *getPosition(*entity);
		})
		.def("setPosition", [](Entity* entity, const Vector3& position) {
			setPosition(*entity, position);
		})
		.def("getOrientation", [](Entity* entity) {
			return *getOrientation(*entity);
		})
		.def("setOrientation", [](Entity* entity, const Quaternion& orientation) {
			setOrientation(*entity, orientation);
		})
		.def("getComponents", &Entity::getComponents)
		.def("getComponentsOfType", &getComponentsOfTypeName)
		.def("getFirstComponentOfType", &getFirstComponentOfTypeName)
		.def("addComponent", &Entity::addComponent)
		.def_property("dynamicsEnabled", &Entity::isDynamicsEnabled, &Entity::setDynamicsEnabled);

	py::class_<World>(m, "World")
		.def("getEntities", &World::getEntities, py::return_value_policy::reference)
		.def("addEntity", &World::addEntity)
		.def("removeEntity", &World::removeEntity)
		.def("removeAllEntities", &World::removeAllEntities);

	py::class_<EntityFactory>(m, "EntityFactory")
		.def("createEntity", &EntityFactory::createEntity, py::return_value_policy::reference);


	m.def("getWorld", &getWorld, "Get default world", py::return_value_policy::reference);
	m.def("setWorld", &setWorld, "Set default world");
	m.def("getEntityFactory", &getEntityFactory, "get default EntityFactory", py::return_value_policy::reference);
	m.def("setEntityFactory", &setEntityFactory, "set default EntityFactory");
	m.def("toGeocentricPosition", [](const PositionPtr& position) { return std::make_shared<GeocentricPosition>(toGeocentric(*position)); });
	m.def("toGeocentricOrientation", [](const OrientationPtr& orientation, const LatLon& latLon) { return std::make_shared<GeocentricOrientation>(toGeocentric(*orientation, latLon)); });
	m.def("toLatLonAlt", [](const PositionPtr& position) { return std::make_shared<LatLonAltPosition>(toLatLonAlt(*position)); });
	m.def("toLatLon", &toLatLon);
	m.def("dot", &dotFunc);
	m.def("cross", &crossFunc);
	m.def("normalize", &normalizeFunc);
	m.def("quaternionFromEuler", py::overload_cast<const Vector3&>(&math::quatFromEuler));
}