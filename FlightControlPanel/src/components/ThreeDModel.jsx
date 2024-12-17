import React from 'react';
import { Canvas, useLoader } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader';
import { MTLLoader } from 'three/examples/jsm/loaders/MTLLoader';
import * as THREE from 'three';


function calculateRotation(gyro) {
  const rotation = {
    x: THREE.MathUtils.degToRad(gyro.GX/70-70),
    y: THREE.MathUtils.degToRad(gyro.GY/70),
    z: THREE.MathUtils.degToRad(gyro.GZ/70-45)
  };
  return rotation;
}



export default function ThreeDModel({gyro}) {
  // Carga el archivo MTL (material)
  const materials = useLoader(MTLLoader, '/model.mtl');

  // Carga el archivo OBJ usando los materiales
  const obj = useLoader(OBJLoader, '/model.obj', (loader) => {
    materials.preload();
    loader.setMaterials(materials);
  });

  // Calcula la rotación del modelo
  obj.rotation.setFromVector3(calculateRotation(gyro));

  return (
    <Canvas>
      {/* Iluminación */}
      <ambientLight intensity={0.5} />
      <directionalLight position={[10, 10, 5]} />
      
      {/* Modelo cargado */}
      <primitive 
        object={obj} 
        scale={0.24} 
        rotation={[
          THREE.MathUtils.degToRad(-70), 
          THREE.MathUtils.degToRad(0), 
          THREE.MathUtils.degToRad(45)
        ]} 
      />
      
      {/* Controles para mover la cámara */}
      <OrbitControls enableDamping={false} enablePan={false} enableZoom={false} />
    </Canvas>
  );
}