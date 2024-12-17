import React, { useState, useEffect } from 'react';
import { RotateCw } from 'lucide-react';
import DroneImage from '../assets/drone.png';
import Prop from '../assets/prop.png';
import ThreeDModel from '../components/ThreeDModel';

const PropellerIcon = ({animate}) => (
  <svg viewBox="0 0 100 100" className="w-16 h-16">
    <circle cx="50" cy="50" r="45" stroke="white" fill="none" strokeWidth="2"/>
    <g transform="translate(55,50)">
      <image className={animate&&'animate-spin'} href={Prop} x="-35" y="-35" width="70" height="52" />
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

  const [gyro, setGyro] = useState({
    AX: null,
    AY: null,
    AZ: null,
    GX: null,
    GY: null,
    GZ: null,
    MX: null,
    MY: null,
    MZ: null
  }); // Estado para almacenar los datos del sensor

  const [socket, setSocket] = useState(null); // Estado para el WebSocket
  const [valorRecibido, setValorRecibido] = useState('Esperando datos...'); // Estado para mensajes recibidos
 
  // State for all dynamic values
  const [propellers, setPropellers] = useState([
    { id: 'PROP 1', speed: 0 },
    { id: 'PROP 2', speed: 0 },
    { id: 'PROP 3', speed: 0 },
    { id: 'PROP 4', speed: 0 },
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
  // useEffect(() => {
  //   const interval = setInterval(() => {
  //     setPropellers(prev => prev.map(prop => ({
  //       ...prop,
  //       speed: Math.min(100, Math.max(0, variation(prop.speed, 10)))
  //     })));
  //     setMasterControl(prev => Math.min(100, Math.max(0, variation(prev, 5))));
  //   }, 1000);
  //   return () => clearInterval(interval);
  // }, []);

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
      // Asumir que queremos usar 'AX' para mover solo en el eje 'x'
      setAxisValues(prev => prev.map((val, index) => ({
        x: gyro.AX ? (gyro.AX/70/1000 / 16385) * 40 : 0, // Ajuste para que el valor se mueva entre 0 y 40
        y: 0 // Mantener 'y' constante
      })));
    }, 200);
  
    return () => clearInterval(interval);
  }, [gyro]);

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

  // useEffect(() => {
  //   const interval = setInterval(() => {
  //     setGraphData(prev => {
  //       // Solo agregamos el valor de GY si es un número válido
  //       const newData = [...prev, gyro.GY/70/1000];
  //       return newData.slice(-50); // Mantener solo los últimos 50 datos
  //     });
  //   }, 200);
  
  //   return () => clearInterval(interval);
  // }, [gyro]);

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



     // WebSocket URL
  const wsUrl = 'ws://192.168.28.125:81'; // Replace with your WebSocket server URL

  // Reconnection parameters
  const [retries, setRetries] = useState(0);
  const maxRetries = 10; // Maximum retry attempts
  const retryDelay = 3000; // Delay in ms between retries

  // Function to create WebSocket connection
  const createWebSocketConnection = () => {
    const ws = new WebSocket(wsUrl);

    ws.onopen = () => {
      console.log('WebSocket connection established');
      setRetries(0); // Reset retry count after successful connection
    };

    ws.onmessage = (event) => {

    console.log('Mensaje recibido:', event.data);

    const data = event.data.split(',').reduce((acc, item) => {
      const [key, value] = item.split(':').map(str => str.trim());
      acc[key] = parseInt(value, 10); // Convertir los valores a enteros
      return acc;
    }, {});

    // Detectar el tipo de datos recibido
    if (data.AX !== undefined || data.GX !== undefined) {
      // Es un mensaje de giroscopio
      setGyro(data);
      console.log('Gyro data:', data);
    } else if (data.M1 !== undefined || data.M2 !== undefined) {
      // Es un mensaje de velocidades de motores
      const updatedPropellers = propellers.map((prop, index) => {
        const motorKey = `M${index + 1}`;
        return {
          ...prop,
          speed: data[motorKey] !== undefined ? data[motorKey]-100 : prop.speed,
        };
      });
      setPropellers(updatedPropellers);
      console.log('Updated propellers:', updatedPropellers);
    } else {
      console.warn('Datos no reconocidos:', data);
    }
  };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    ws.onclose = () => {
      console.log('WebSocket connection closed');
      if (retries < maxRetries) {
        setRetries(prevRetries => prevRetries + 1);
        setTimeout(createWebSocketConnection, retryDelay); // Try to reconnect after a delay
      } else {
        console.log('Max retries reached, WebSocket will not reconnect');
      }
    };

    setSocket(ws); // Save WebSocket instance in state
  };

  // Initialize WebSocket connection on component mount
  useEffect(() => {
    createWebSocketConnection(); // Initial WebSocket connection attempt

    return () => {
      if (socket && socket.readyState === WebSocket.OPEN) {
        socket.close(); // Clean up WebSocket when component unmounts
      }
    };
  }, []); // Empty array to run this effect only once on mount
  
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
                        <PropellerIcon animate={prop.speed>100} />
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
              onClick={() => sendCommand(e.target.value)}
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
              <ThreeDModel gyro={gyro} />
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