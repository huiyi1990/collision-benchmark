#include <collision_benchmark/GazeboWorldState.hh>

#include <gazebo/common/common.hh>
#include <gazebo/physics/physics.hh>


/**
 * Helper function which fixes the SDF format in the states description.
 *  adds the <sdf version='1.6'>...</sdf> tags around the strings which have to
 *  be there for compatibility with World::SetState(). The insertions coming
 *  from WorldState::operator- are in a format not compatible with it.
 */
void fixSDF(std::vector<std::string>& states)
{
  for (std::vector<std::string>::iterator it = states.begin(); it != states.end(); ++it)
  {
    std::stringstream mod;
    mod << "<sdf version='1.6'>" << *it << "</sdf>";
    *it = std::string(mod.str());
  }
}

/**
 * Returns new entities which were added in \e state2 when compared to _state1
 * \param models new models added in \e state2
 * \param models new lights added in \e state2
 */
void GetNewEntities(const gazebo::physics::WorldState& _state1,
          const gazebo::physics::WorldState& _state2,
          std::vector<gazebo::physics::ModelState>& models,
          std::vector<gazebo::physics::LightState>& lights)
{
  const gazebo::physics::ModelState_M& _modelStates1 = _state1.GetModelStates();
  for (gazebo::physics::ModelState_M::const_iterator iter =
        _modelStates1.begin(); iter != _modelStates1.end(); ++iter)
  {
    if (!_state2.HasModelState(iter->second.GetName()))
    {
      models.push_back(iter->second);
    }
  }

  const gazebo::physics::LightState_M& _lightStates1 = _state1.LightStates();
  for (const auto & light : _lightStates1)
  {
    if (!_state2.HasLightState(light.second.GetName()))
    {
      lights.push_back(light.second);
    }
  }
}

// XXX TODO REMOVE: Flags for testing
#define FORCE_TARGET_TIME_VALUES
//#define FORCE_KEEP_TIME_VALUES
//#define DEBUGMIRR
void collision_benchmark::SetWorldState(gazebo::physics::WorldPtr& world, const gazebo::physics::WorldState& targetState)
{
  bool pauseState = world->IsPaused();
  world->SetPaused(true);
  gazebo::physics::WorldState currentState(world);

#ifdef DEBUGMIRR
  std::cout << "Setting world state. " << std::endl;
  std::cout << "Target state: " << std::endl << targetState << std::endl;
  std::cout << "Current state: " << std::endl << currentState << std::endl;
#endif

  //re-set mirror state times
  // XXX TODO CHECK: should reset all times of the state as well,
  // because the *difference* is going to be added to it
  // Not sure yet how to best handle the times.
  /*  currentState.SetWallTime(common::Time(0));
    currentState.SetRealTime(common::Time(0));
    currentState.SetIterations(0);*/

  // Handle all insertions/deletions of models in a
  // differential state. The result will have the name of
  // origState, adding all the models/lights/etc from origState which are
  // not in emptyState (which is empty so it will be all models).
  gazebo::physics::WorldState diffState = targetState - currentState;
  gazebo::physics::WorldState newState = currentState + diffState;

#ifdef DEBUGMIRR
  std::cout << "Diff state: " << std::endl << diffState << std::endl;
#endif

  // Force the new state to use certain iteration/time values
  // in order to maintain consistency within the world

#ifdef FORCE_TARGET_TIME_VALUES
  // the + operator doesn't add iterations and times
  newState.SetIterations(targetState.GetIterations());
  newState.SetWallTime(targetState.GetWallTime());
  newState.SetRealTime(targetState.GetRealTime());
  newState.SetSimTime(targetState.GetSimTime());
#endif

#ifdef FORCE_KEEP_TIME_VALUES
  // the + operator doesn't add iterations and times
  newState.SetIterations(currentState.GetIterations());
  newState.SetWallTime(currentState.GetWallTime());
  newState.SetRealTime(currentState.GetRealTime());
  newState.SetSimTime(currentState.GetSimTime());
#endif

  std::vector<std::string> insertions = diffState.Insertions();
  fixSDF(insertions);
  newState.SetInsertions(insertions);
  std::vector<std::string> deletions = diffState.Deletions();
  newState.SetDeletions(deletions);
  fixSDF(deletions);

  // the insertions still have one drawback: They don't contain
  // the current model pose. So subsequently, the current pose
  // is only published as message in the *next* world update *only if*
  // the pose has changed (from within Model::OnPoseChange by call of World::SetWorldPose).
  // But if the pose within the target state world has not changed,
  // no message is published. So we need to force publishing the poses
  // of the newly inserted models.
  std::vector<gazebo::physics::ModelState> models;
  std::vector<gazebo::physics::LightState> lights;
  GetNewEntities(targetState, currentState, models, lights);

  // apply the state to the mirror world
  world->SetState(newState);

  // now, update the poses of the new models and lights
  for (const auto & model : models)
  {
    // std::cout<<"New model: "<<model.GetName()<<std::endl;
    gazebo::physics::ModelPtr m = world->GetModel(model.GetName());
    if (!m)
    {
      throw new gazebo::common::Exception(__FILE__, __LINE__,
                        "Model not found though it should have been inserted.");
    }
    m->SetState(model);
  }

  for (const auto & light : lights)
  {
    // std::cout<<"New light: "<<light.GetName()<<std::endl;
    gazebo::physics::LightPtr l = world->Light(light.GetName());
    if (!l)
    {
      throw new gazebo::common::Exception(__FILE__, __LINE__,
                        "Light not found though it should have been inserted.");
    }
    l->SetState(light);
  }

#ifdef DEBUGMIRR
  std::cout << "State set to:" << std::endl;
  gazebo::physics::WorldState _currentState(world);
  std::cout << _currentState << std::endl;
#endif

  world->SetPaused(pauseState);
}


void collision_benchmark::PrintWorldState(const gazebo::physics::WorldPtr world)
{
  std::cout << "## State of world " << world->GetName() << std::endl;
  gazebo::physics::WorldState _state(world);
  std::cout << _state << std::endl;
}

void collision_benchmark::PrintWorldStates(const std::vector<gazebo::physics::WorldPtr>& worlds)
{
  std::cout << "## World states ###" << std::endl;
  for (std::vector<gazebo::physics::WorldPtr>::const_iterator w = worlds.begin();
      w != worlds.end(); ++w)
  {
    PrintWorldState(*w);
  }
  std::cout << "#####" << std::endl;
}


