/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2016 Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): Sebastian Barschkis (sebbas)
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file mantaflow/intern/strings/liquid.h
 *  \ingroup mantaflow
 */

#include <string>

//////////////////////////////////////////////////////////////////////
// BOUNDS
//////////////////////////////////////////////////////////////////////

const std::string liquid_bounds_low = "\n\
# prepare domain low\n\
mantaMsg('Liquid domain low')\n\
flags.initDomain(boundaryWidth=boundaryWidth, phiWalls=phiObs)\n\
if doOpen:\n\
    setOpenBound(flags=flags, bWidth=boundaryWidth, openBound=boundConditions, type=FlagOutflow|FlagEmpty)\n";

const std::string liquid_bounds_high = "\n\
# prepare domain high\n\
mantaMsg('Liquid domain high')\n\
xl_flags.initDomain(boundaryWidth=boundaryWidth, phiWalls=phiObs)\n\
if doOpen:\n\
    setOpenBound(flags=xl_flags, bWidth=boundaryWidth, openBound=boundConditions, type=FlagOutflow|FlagEmpty)\n";

//////////////////////////////////////////////////////////////////////
// VARIABLES
//////////////////////////////////////////////////////////////////////

const std::string liquid_variables_low = "\n\
mantaMsg('Liquid variables low')\n\
narrowBandWidth  = 3\n\
combineBandWidth = narrowBandWidth - 1\n\
\n\
particleNumber = $PARTICLE_NUMBER$\n\
minParticles   = pow(particleNumber,dim)\n\
radiusFactor   = $PARTICLE_RADIUS$\n\
randomness     = $PARTICLE_RANDOMNESS$\n\
\n\
maxVel  = 0\n\
\n\
using_highres = $USING_HIGHRES$\n";

const std::string liquid_variables_high = "\n\
mantaMsg('Liquid variables high')\n";

//////////////////////////////////////////////////////////////////////
// GRIDS & MESH & PARTICLESYSTEM
//////////////////////////////////////////////////////////////////////

const std::string liquid_alloc_low = "\n\
mantaMsg('Liquid alloc low')\n\
flags      = s.create(FlagGrid)\n\
\n\
phiParts   = s.create(LevelsetGrid)\n\
phi        = s.create(LevelsetGrid)\n\
phiInit    = s.create(LevelsetGrid)\n\
pressure   = s.create(RealGrid)\n\
\n\
phiObs     = s.create(LevelsetGrid)\n\
phiObsInit = s.create(LevelsetGrid)\n\
fractions  = s.create(MACGrid)\n\
\n\
vel        = s.create(MACGrid)\n\
x_vel      = s.create(RealGrid)\n\
y_vel      = s.create(RealGrid)\n\
z_vel      = s.create(RealGrid)\n\
obvel      = s.create(MACGrid)\n\
x_obvel    = s.create(RealGrid)\n\
y_obvel    = s.create(RealGrid)\n\
z_obvel    = s.create(RealGrid)\n\
velOld     = s.create(MACGrid)\n\
velParts   = s.create(MACGrid)\n\
mapWeights = s.create(MACGrid)\n\
\n\
pp         = s.create(BasicParticleSystem)\n\
pVel       = pp.create(PdataVec3)\n\
mesh       = s.create(Mesh)\n\
\n\
# Acceleration data for particle nbs\n\
pindex     = s.create(ParticleIndexSystem)\n\
gpi        = s.create(IntGrid)\n\
\n\
forces     = s.create(MACGrid)\n\
x_force    = s.create(RealGrid)\n\
y_force    = s.create(RealGrid)\n\
z_force    = s.create(RealGrid)\n";

const std::string liquid_alloc_high = "\n\
mantaMsg('Liquid alloc high')\n\
xl_flags    = xl.create(FlagGrid)\n\
xl_phiParts = xl.create(LevelsetGrid)\n\
xl_phi      = xl.create(LevelsetGrid)\n\
xl_pp       = xl.create(BasicParticleSystem)\n\
xl_mesh     = xl.create(Mesh)\n\
\n\
# Acceleration data for particle nbs\n\
xl_pindex  = xl.create(ParticleIndexSystem)\n\
xl_gpi     = xl.create(IntGrid)\n";

const std::string liquid_init_phi = "\n\
phi.initFromFlags(flags)\n\
phiInit.initFromFlags(flags)\n";

//////////////////////////////////////////////////////////////////////
// STEP FUNCTIONS
//////////////////////////////////////////////////////////////////////

const std::string liquid_adaptive_step = "\n\
def manta_step(start_frame):\n\
    s.frame = start_frame\n\
    s.timeTotal = s.frame * dt0\n\
    last_frame = s.frame\n\
    \n\
    if start_frame == 1:\n\
        phi.join(phiInit)\n\
        phiObs.join(phiObsInit)\n\
        \n\
        flags.updateFromLevelset(phi)\n\
        phi.subtract(phiObs)\n\
        \n\
        sampleLevelsetWithParticles(phi=phi, flags=flags, parts=pp, discretization=particleNumber, randomness=randomness)\n\
        \n\
        updateFractions(flags=flags, phiObs=phiObs, fractions=fractions, boundaryWidth=boundaryWidth)\n\
        setObstacleFlags(flags=flags, phiObs=phiObs, fractions=fractions)\n\
    \n\
    while s.frame == last_frame:\n\
        sampleLevelsetWithParticles(phi=phiInit, flags=flags, parts=pp, discretization=particleNumber, randomness=randomness, refillEmpty=True)\n\
        \n\
        mantaMsg('Adapt timestep')\n\
        maxvel = vel.getMaxValue()\n\
        s.adaptTimestep(maxvel)\n\
        \n\
        mantaMsg('Low step / s.frame: ' + str(s.frame))\n\
        liquid_step()\n\
        \n\
        if using_highres:\n\
            xl.timestep = s.timestep\n\
            mantaMsg('High step / s.frame: ' + str(s.frame))\n\
            liquid_step_high()\n\
        \n\
        s.step()\n";

const std::string liquid_step_low = "\n\
def liquid_step():\n\
    mantaMsg('Liquid step low')\n\
    copyRealToVec3(sourceX=x_vel, sourceY=y_vel, sourceZ=z_vel, target=vel)\n\
    copyRealToVec3(sourceX=x_obvel, sourceY=y_obvel, sourceZ=z_obvel, target=obvel)\n\
    \n\
    # FLIP\n\
    pp.advectInGrid(flags=flags, vel=vel, integrationMode=IntRK4, deleteInObstacle=False, stopInObstacle=False)\n\
    pushOutofObs(parts=pp, flags=flags, phiObs=phiObs)\n\
    \n\
    advectSemiLagrange(flags=flags, vel=vel, grid=phi, order=1, openBounds=doOpen, boundaryWidth=boundaryWidth) # first order is usually enough\n\
    advectSemiLagrange(flags=flags, vel=vel, grid=vel, order=2, openBounds=doOpen, boundaryWidth=boundaryWidth)\n\
    \n\
    # Keep an original copy of interpolated phi grid for later use in (optional) high-res step\n\
    if using_highres:\n\
        interpolateGrid(target=xl_phi, source=phi)\n\
    \n\
    # create level set of particles\n\
    gridParticleIndex(parts=pp , flags=flags, indexSys=pindex, index=gpi)\n\
    unionParticleLevelset(pp, pindex, flags, gpi, phiParts)\n\
    \n\
    # combine level set of particles with grid level set\n\
    phi.addConst(1.) # shrink slightly\n\
    phi.join(phiParts)\n\
    extrapolateLsSimple(phi=phi, distance=narrowBandWidth+2, inside=True)\n\
    extrapolateLsSimple(phi=phi, distance=3)\n\
    phi.setBoundNeumann(1) # make sure no particles are placed at outer boundary\n\
    if doOpen:\n\
        resetOutflow(flags=flags, phi=phi, parts=pp, index=gpi, indexSys=pindex) # open boundaries\n\
    flags.updateFromLevelset(phi)\n\
    \n\
    # combine particles velocities with advected grid velocities\n\
    mapPartsToMAC(vel=velParts, flags=flags, velOld=velOld, parts=pp, partVel=pVel, weight=mapWeights)\n\
    extrapolateMACFromWeight(vel=velParts, distance=2, weight=mapWeights)\n\
    combineGridVel(vel=velParts, weight=mapWeights, combineVel=vel, phi=phi, narrowBand=combineBandWidth, thresh=0)\n\
    velOld.copyFrom(vel)\n\
    \n\
    # forces & pressure solve\n\
    addGravity(flags=flags, vel=vel, gravity=gravity)\n\
    copyRealToVec3(sourceX=x_force, sourceY=y_force, sourceZ=z_force, target=forces)\n\
    addForceField(flags=flags, vel=vel, force=forces)\n\
    forces.clear()\n\
    \n\
    extrapolateMACSimple(flags=flags, vel=vel, distance=2, intoObs=True)\n\
    setWallBcs(flags=flags, vel=vel, fractions=fractions, phiObs=phiObs)\n\
    \n\
    solvePressure(flags=flags, vel=vel, pressure=pressure, phi=phi, fractions=fractions)\n\
    \n\
    extrapolateMACSimple(flags=flags, vel=vel, distance=4, intoObs=True)\n\
    setWallBcs(flags=flags, vel=vel, fractions=fractions, phiObs=phiObs)\n\
    \n\
    if (dim==3):\n\
        # mis-use phiParts as temp grid to close the mesh\n\
        phiParts.copyFrom(phi)\n\
        phiParts.setBound(0.5,0)\n\
        phiParts.createMesh(mesh)\n\
	\n\
    # set source grids for resampling, used in adjustNumber!\n\
    pVel.setSource(vel, isMAC=True)\n\
    adjustNumber(parts=pp, vel=vel, flags=flags, minParticles=1*minParticles, maxParticles=2*minParticles, phi=phi, exclude=phiObs, radiusFactor=radiusFactor, narrowBand=narrowBandWidth)\n\
    flipVelocityUpdate(vel=vel, velOld=velOld, flags=flags, parts=pp, partVel=pVel, flipRatio=0.95)\n\
    \n\
    # TODO (sebbas): HACK - saving particle system for highres step\n\
    if using_highres:\n\
        pp.save(os.path.join(tempfile.gettempdir(), 'partfile.uni'))\n\
    \n\
    copyVec3ToReal(source=vel, targetX=x_vel, targetY=y_vel, targetZ=z_vel)\n\
    copyVec3ToReal(source=obvel, targetX=x_obvel, targetY=y_obvel, targetZ=z_obvel)\n";

const std::string liquid_step_high = "\n\
def liquid_step_high():\n\
    mantaMsg('Liquid step high')\n\
    xl_pp.load(os.path.join(tempfile.gettempdir(), 'partfile.uni'))\n\
    \n\
    # create surface\n\
    gridParticleIndex( parts=xl_pp , flags=xl_flags, indexSys=xl_pindex, index=xl_gpi )\n\
    #unionParticleLevelset( xl_pp, xl_pindex, xl_flags, xl_gpi, xl_phi , radiusFactor ) # faster, but not as smooth\n\
    averagedParticleLevelset( xl_pp, xl_pindex, xl_flags, xl_gpi, xl_phiParts, radiusFactor , 1, 1 )\n\
    xl_phi.join(xl_phiParts)\n\
    \n\
    xl_phi.createMesh(xl_mesh)\n";

//////////////////////////////////////////////////////////////////////
// IMPORT / EXPORT
//////////////////////////////////////////////////////////////////////

const std::string liquid_save_mesh_low = "\n\
def save_mesh_low(path):\n\
    mesh.save(path)\n";

const std::string liquid_save_mesh_high = "\n\
def save_mesh_high(path):\n\
    xl_mesh.save(path)\n";

const std::string liquid_import_low = "\n\
def load_liquid_data_low(path):\n\
    flags.load(os.path.join(path, 'flags.uni'))\n\
    \n\
    phiParts.load(os.path.join(path, 'phiParts.uni'))\n\
    phi.load(os.path.join(path, 'phi.uni'))\n\
    phiInit.load(os.path.join(path, 'phiInit.uni'))\n\
    phiObs.load(os.path.join(path, 'phiObs.uni'))\n\
    phiObsInit.load(os.path.join(path, 'phiObsInit.uni'))\n\
    fractions.load(os.path.join(path, 'fractions.uni'))\n\
    pressure.load(os.path.join(path, 'pressure.uni'))\n\
    \n\
    vel.load(os.path.join(path, 'vel.uni'))\n\
    velOld.load(os.path.join(path, 'velOld.uni'))\n\
    velParts.load(os.path.join(path, 'velParts.uni'))\n\
    mapWeights.load(os.path.join(path, 'mapWeights.uni'))\n\
    \n\
    pp.load(os.path.join(path, 'pp.uni'))\n\
    pVel.load(os.path.join(path, 'pVel.uni'))\n\
    \n\
    gpi.load(os.path.join(path, 'gpi.uni'))\n";

const std::string liquid_import_high = "\n\
def load_liquid_data_high(path):\n\
    xl_flags.load(os.path.join(path, 'xl_flags.uni'))\n\
    \n\
    xl_phiParts.load(os.path.join(path, 'xl_phiParts.uni'))\n\
    xl_phi.load(os.path.join(path, 'xl_phi.uni'))\n\
    \n\
    xl_pp.load(os.path.join(path, 'xl_pp.uni'))\n";

const std::string liquid_export_low = "\n\
def save_liquid_data_low(path):\n\
    flags.save(os.path.join(path, 'flags.uni'))\n\
    \n\
    phiParts.save(os.path.join(path, 'phiParts.uni'))\n\
    phi.save(os.path.join(path, 'phi.uni'))\n\
    phiInit.save(os.path.join(path, 'phiInit.uni'))\n\
    phiObs.save(os.path.join(path, 'phiObs.uni'))\n\
    phiObsInit.save(os.path.join(path, 'phiObsInit.uni'))\n\
    fractions.save(os.path.join(path, 'fractions.uni'))\n\
    pressure.save(os.path.join(path, 'pressure.uni'))\n\
    \n\
    vel.save(os.path.join(path, 'vel.uni'))\n\
    velOld.save(os.path.join(path, 'velOld.uni'))\n\
    velParts.save(os.path.join(path, 'velParts.uni'))\n\
    mapWeights.save(os.path.join(path, 'mapWeights.uni'))\n\
    \n\
    pp.save(os.path.join(path, 'pp.uni'))\n\
    pVel.save(os.path.join(path, 'pVel.uni'))\n\
    \n\
    gpi.save(os.path.join(path, 'gpi.uni'))\n";

const std::string liquid_export_high = "\n\
def save_liquid_data_high(path):\n\
    xl_flags.save(os.path.join(path, 'xl_flags.uni'))\n\
    \n\
    xl_phiParts.save(os.path.join(path, 'xl_phiParts.uni'))\n\
    xl_phi.save(os.path.join(path, 'xl_phi.uni'))\n\
    \n\
    xl_pp.save(os.path.join(path, 'xl_pp.uni'))\n";

//////////////////////////////////////////////////////////////////////
// DESTRUCTION
//////////////////////////////////////////////////////////////////////

const std::string liquid_delete_grids_low = "\n\
mantaMsg('Deleting lowres grids, mesh, particlesystem')\n\
if 'flags'      in globals() : del flags\n\
if 'phiParts'   in globals() : del phiParts\n\
if 'phi'        in globals() : del phi\n\
if 'phiInit'    in globals() : del phiInit\n\
if 'pressure'   in globals() : del pressure\n\
if 'vel'        in globals() : del vel\n\
if 'x_vel'      in globals() : del x_vel\n\
if 'y_vel'      in globals() : del y_vel\n\
if 'z_vel'      in globals() : del z_vel\n\
if 'obvel'      in globals() : del obvel\n\
if 'x_obvel'    in globals() : del x_obvel\n\
if 'y_obvel'    in globals() : del y_obvel\n\
if 'z_obvel'    in globals() : del z_obvel\n\
if 'velOld'     in globals() : del velOld\n\
if 'velParts'   in globals() : del velParts\n\
if 'mapWeights' in globals() : del mapWeights\n\
if 'pp'         in globals() : del pp\n\
if 'pVel'       in globals() : del pVel\n\
if 'mesh'       in globals() : del mesh\n\
if 'pindex'     in globals() : del pindex\n\
if 'gpi'        in globals() : del gpi\n\
if 'forces'     in globals() : del forces\n\
if 'x_force'    in globals() : del x_force\n\
if 'y_force'    in globals() : del y_force\n\
if 'z_force'    in globals() : del z_force\n\
if 'phiObs'     in globals() : del phiObs\n\
if 'phiObsInit' in globals() : del phiObsInit\n\
if 'fractions'  in globals() : del fractions\n";

const std::string liquid_delete_grids_high = "\n\
mantaMsg('Deleting highres grids, mesh, particlesystem')\n\
if 'xl_flags'    in globals() : del xl_flags\n\
if 'xl_phiParts' in globals() : del xl_phiParts\n\
if 'xl_phi'      in globals() : del xl_phi\n\
if 'xl_pp'       in globals() : del xl_pp\n\
if 'xl_mesh'     in globals() : del xl_mesh\n\
if 'xl_pindex'   in globals() : del xl_pindex\n\
if 'xl_gpi'      in globals() : del xl_gpi\n";

const std::string liquid_delete_variables_low = "\n\
mantaMsg('Deleting lowres liquid variables')\n\
if 'narrowBandWidth'  in globals() : del narrowBandWidth\n\
if 'combineBandWidth' in globals() : del combineBandWidth\n\
if 'minParticles'     in globals() : del minParticles\n\
if 'particleNumber'   in globals() : del particleNumber\n\
if 'maxVel'           in globals() : del maxVel\n";

const std::string liquid_delete_variables_high = "\n\
mantaMsg('Deleting highres liquid variables')\n";

//////////////////////////////////////////////////////////////////////
// STANDALONE MODE
//////////////////////////////////////////////////////////////////////

const std::string liquid_standalone_load = "\n\
# import *.uni files\n\
path_prefix = '$MANTA_EXPORT_PATH$'\n\
load_liquid_data_low(path_prefix)\n\
if using_highres:\n\
    load_liquid_data_high(path_prefix)\n";


