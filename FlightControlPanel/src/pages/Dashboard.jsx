import React, { useState, useEffect } from 'react';
import { RotateCw } from 'lucide-react';
import DroneImage from '../assets/drone.png';
import Prop from '../assets/prop.png';
import ThreeDModel from '../components/ThreeDModel';

const PropellerIcon = () => (
  <svg viewBox="0 0 100 100" className="w-16 h-16">
    <circle cx="50" cy="50" r="45" stroke="white" fill="none" strokeWidth="2"/>
    <g transform="translate(55,50)">
      <image className='animate-spin' href={Prop} x="-35" y="-35" width="70" height="52" />
    </g>
  </svg>
);

const PowerBar = ({ value }) => (
  <div className="h-32 w-3 bg-neutral-800 relative rounded-sm overflow-hidden">
    <div 
      className="absolute bottom-0 left-0 right-0 bg-green-500 transition-all duration-300"
      style={{ height: `${value}%` }}
    />
  </div>
);

const SteppedPowerBar = ({ value, onIncrease, onDecrease }) => (
  <div className="flex items-center space-x-2">
    <button 
      onClick={onDecrease} 
      className="bg-neutral-700 text-white px-2 py-1 rounded hover:bg-neutral-600"
    >
      ▼
    </button>
    <div className="h-32 w-8 bg-neutral-800 relative rounded-sm overflow-hidden">
      {[...Array(20)].map((_, index) => (
        <div 
          key={index} 
          className="absolute w-full border-b border-black"
          style={{ 
            bottom: `${index * 5}%`, 
            opacity: index * 5 <= value ? 1 : 0.3 
          }}
        />
      ))}
      <div 
        className="absolute bottom-0 left-0 right-0 bg-green-500 transition-all duration-300"
        style={{ height: `${value}%` }}
      />
    </div>
    <button 
      onClick={onIncrease} 
      className="bg-neutral-700 text-white px-2 py-1 rounded hover:bg-neutral-600"
    >
      ▲
    </button>
  </div>
);

const Dashboard = () => {
  const [socket, setSocket] = useState(null); // Estado para el WebSocket
  const [valorRecibido, setValorRecibido] = useState('Esperando datos...'); // Estado para mensajes recibidos
 
  // State for all dynamic values
  const [propellers, setPropellers] = useState([
    { id: 'PROP 1', speed: 65 },
    { id: 'PROP 2', speed: 65 },
    { id: 'PROP 3', speed: 55 },
    { id: 'PROP 4', speed: 55 },
  ]);
  
  const [masterControl, setMasterControl] = useState(55);
  const [horizontalSpeed, setHorizontalSpeed] = useState(54);
  const [verticalSpeed, setVerticalSpeed] = useState(54);
  const [customControl, setCustomControl] = useState(0);
  const [axisValues, setAxisValues] = useState(
    Array(9).fill().map(() => ({ x: 0, y: 0 }))
  );
  const [graphData, setGraphData] = useState([]);
  
  // Function to generate random variations
  const variation = (base, range) => base + (Math.random() - 0.5) * range;
  
  // Update propeller speeds
  useEffect(() => {
    const interval = setInterval(() => {
      setPropellers(prev => prev.map(prop => ({
        ...prop,
        speed: Math.min(100, Math.max(0, variation(prop.speed, 10)))
      })));
      setMasterControl(prev => Math.min(100, Math.max(0, variation(prev, 5))));
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  // Update speeds
  useEffect(() => {
    const interval = setInterval(() => {
      setHorizontalSpeed(prev => Math.max(0, variation(prev, 8)));
      setVerticalSpeed(prev => Math.max(0, variation(prev, 8)));
    }, 500);
    return () => clearInterval(interval);
  }, []);

  // Update axis positions
  useEffect(() => {
    const interval = setInterval(() => {
      setAxisValues(prev => prev.map(val => ({
        x: variation(val.x, 0.4),
        y: variation(val.y, 0.4)
      })));
    }, 200);
    return () => clearInterval(interval);
  }, []);

  // Update graph data
  useEffect(() => {
    const interval = setInterval(() => {
      setGraphData(prev => {
        const newData = [...prev, variation(50, 20)];
        return newData.slice(-50);
      });
    }, 200);
    return () => clearInterval(interval);
  }, []);

  // Custom Control Handlers
  const handleCustomIncrease = () => {
    setCustomControl(prev => Math.min(100, prev + 5));
    sendCommand(`PWR${customControl}`); // Enviar comando al ESP8266
  };

  const handleCustomDecrease = () => {
    setCustomControl(prev => Math.max(0, prev - 5));
    sendCommand(`PWR${customControl}`); // Enviar comando al ESP8266
  };

  // Axis labels
  const axisControls = [
    ['X', 'Y', 'Z'],
    ['U', 'V', 'W'],
    ['A', 'B', 'C']
  ];



    // Inicializar WebSocket
    useEffect(() => {
      const ws = new WebSocket('ws://192.168.235.45:81'); // Cambia la IP por la de tu ESP8266
      setSocket(ws);
  
      ws.onopen = () => {
        console.log('Conexión WebSocket abierta');
      };
  
      ws.onmessage = (event) => {
        console.log('Mensaje recibido:', event.data);
        setValorRecibido(event.data); // Actualiza el estado con el mensaje recibido
      };
  
      ws.onerror = (error) => {
        console.error('Error en WebSocket:', error);
      };
  
      ws.onclose = () => {
        console.log('Conexión WebSocket cerrada');
      };
  
      // Limpieza al desmontar el componente
      return () => {
        ws.close();
      };
    }, []);
  
    // Función para enviar mensajes
    const sendCommand = (command) => {
      if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(command);
        console.log('Mensaje enviado:', command);
      } else {
        console.warn('WebSocket no está conectado');
      }
    };

  return (
    <div className="bg-black min-h-screen p-8 text-white">
      <h1 className="text-2xl font-bold mb-8">DRONE CONTROL</h1>
      
      <div className="grid grid-cols-3 gap-8">
        {/* Left Column - Propellers and Speed */}
        <div className="space-y-6">
          {/* Propellers Section */}
          <div className="bg-neutral-900 p-6 rounded-lg justify-center flex">
            <div className='items-center flex flex-row '>
              <div className="grid grid-cols-2 gap-x-12 gap-y-8">
                {propellers.map((prop, index) => (
                  <div key={index} className="flex items-start gap-4">
                    <div className="flex flex-col items-center">
                      <span className="text-xs mb-1">{prop.id}</span>
                      <div className="bg-black rounded-full p-1">
                        <PropellerIcon />
                      </div>
                    </div>
                    <div className="flex flex-col items-center">
                      <PowerBar value={prop.speed} />
                      <span className="text-xs mt-1">{prop.speed.toFixed(0)}%</span>
                    </div>
                  </div>
                ))}
              </div>
              
              {/* Master Control */}
              <div className="flex justify-end mt-4 mr-4 ml-10 space-x-4">
                <div className="flex flex-col items-center">
                  <PowerBar value={masterControl} />
                  <span className="text-xs mt-1">MASTER</span>
                  <span className="text-xs">{masterControl.toFixed(0)}%</span>
                </div>

                {/* New Custom Control */}
                <div className="flex flex-col items-center">
                  <SteppedPowerBar 
                    value={customControl} 
                    onIncrease={handleCustomIncrease}
                    onDecrease={handleCustomDecrease}
                  />
                  <span className="text-xs mt-1">CONTROL</span>
                  <span className="text-xs">{customControl.toFixed(0)}%</span>
                </div>
              </div>
            </div>
          </div>

          {/* Speed Gauges */}
          <div className="bg-neutral-900 p-4 rounded-lg">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <div className="text-sm mb-2">HORIZONTAL SPEED</div>
                <div className="relative h-24 w-full">
                  <div className="absolute inset-0 flex items-center justify-center">
                    <div className="text-xl font-bold">{horizontalSpeed.toFixed(1)}k/h</div>
                  </div>
                </div>
              </div>
              <div>
                <div className="text-sm mb-2">VERTICAL SPEED</div>
                <div className="relative h-24 w-full">
                  <div className="absolute inset-0 flex items-center justify-center">
                    <div className="text-xl font-bold">{verticalSpeed.toFixed(1)}k/h</div>
                  </div>
                </div>
              </div> 
            </div>
          </div>


          {/* WebSocket Messages */}
          <div className="bg-neutral-900 p-6 rounded-lg">
            <h2 className="text-xl font-bold mb-4">COMMAND SEND</h2>
            <div className="mb-4">
              <span>Valor recibido en tiempo real: </span>
              <span className="font-bold">{valorRecibido}</span>
            </div>
            <div className='flex flex-row space-x-3'>
            <input
              type="text"
              className="w-full p-2 bg-neutral-800 rounded mb-2"
              placeholder="Send a command to the drone"
              onKeyDown={(e) => {
               if (e.key === 'Enter') sendCommand(e.target.value);
              }}
            />
            <button
              onClick={() => sendCommand('Hola desde React')}
              className="bg-green-600 px-4  rounded hover:bg-green-500"
            >
              Send Command
            </button>
            </div>
          </div>



        </div>

        {/* Middle Column - Drone View and Controls */}
        <div className="flex flex-col items-center justify-center space-y-8">
          {/* Drone Image */}
          <div className="">
            <div style={{ width: '500px', height: '400px' }}>
              <ThreeDModel />
            </div>
          </div>

          {/* Direction Controls */}
          <div className="grid grid-cols-3 gap-2">
            <div />
            <button className="bg-gray-700 p-4 rounded hover:bg-gray-600 transition-colors">↑</button>
            <div />
            <button className="bg-gray-700 p-4 rounded hover:bg-gray-600 transition-colors">←</button>
            <button className="bg-gray-700 p-4 rounded hover:bg-gray-600 transition-colors">↓</button>
            <button className="bg-gray-700 p-4 rounded hover:bg-gray-600 transition-colors">→</button>
          </div>
        </div>

        {/* Right Column - Fixed Axis Controls alignment */}
        <div className="bg-neutral-900 p-4 rounded-lg">
          <div className="grid grid-cols-3 gap-4">
            {axisControls.flat().map((axis, index) => (
              <div key={index} className="flex flex-col items-center">
                <div className="text-sm mb-2">AXIS {axis}</div>
                <div className="w-16 h-16 rounded-full border-2 border-gray-700 relative mb-2">
                  <div 
                    className="absolute w-2 h-2 bg-red-500 rounded-full transition-all duration-200"
                    style={{
                      left: `${50 + axisValues[index].x * 40}%`,
                      top: `${50 + axisValues[index].y * 40}%`,
                      transform: 'translate(-50%, -50%)'
                    }}
                  />
                </div>
                <div className="text-xs text-center">
                  x: {axisValues[index].x.toFixed(2)}<br />
                  y: {axisValues[index].y.toFixed(2)}
                </div>
              </div>
            ))}
          </div>
          
          {/* Graph */}
          <div className="mt-8">
            <div className="text-sm mb-2">Alguna Cosa</div>
            <div className="bg-neutral-800 h-32 rounded-lg p-2">
              <svg viewBox="0 0 100 100" className="w-full h-full" preserveAspectRatio="none">
                <path
                  d={`M ${graphData.map((value, index) => 
                    `${(index / (graphData.length - 1)) * 100},${value}`
                  ).join(' L ')}`}
                  fill="none"
                  stroke="white"
                  strokeWidth="1"
                  vectorEffect="non-scaling-stroke"
                />
              </svg>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default Dashboard;