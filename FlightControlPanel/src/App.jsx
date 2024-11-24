import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import Dashboard from './pages/Dashboard'

function App() {

  return (
  <Router>
    <Routes>
        <Route path="/" element={<Dashboard/>} />
        <Route path='*' element={<h1 className='text-5xl text-white text-center mt-56'>404 Not Found :(</h1>} />
    </Routes>
  </Router>
  )
}

export default App
