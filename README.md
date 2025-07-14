# ParamEQ

A lightweight and customizable parametric equalizer plugin built using JUCE and C++.  
Designed for real-time audio processing in VST3-compatible DAWs.

---

## Features

- 8 fully independent EQ bands  
- Filter types: Peak, Low Shelf, High Shelf, Low-pass, High-pass  
- Real-time spectrum analyzer and EQ curve display  
- Responsive and optimized UI  
- Stereo audio processing  
- Full VST3 host automation support  
- Preset saving/restoration via DAW session state  

---

## 🌿 Branches

### `main`

- 🔄 Default development branch  
- 🛠️ Active development happens here  
- 💬 Open to pull requests and community contributions (requires approval)

### `tcc-final`

- 📌 Snapshot of the exact state of the project at the time of its submission as a **final academic project**  
- 🧊 Permanently frozen and protected from changes  
- 📎 Serves as an academic reference version

---

## Building the Plugin

This project uses JUCE (v8.0.8) and CMake for building the plugin. It is configured to automatically download JUCE as a dependency.

### 🔧 Requirements

- CMake 3.15 or later  
- C++17-compatible compiler (GCC, Clang or MSVC)  
- Git  
- JUCE-compatible DAW for testing (e.g., Ableton Live, Reaper, AudioPluginHost)  

### 📦 Build steps (Windows/Linux/macOS)

1. Clone the repository:

   ```bash
   git clone --recursive https://github.com/your-user/ParamEQ.git
   cd ParamEQ
   ```

2. Create a build directory and run CMake:

   ```bash
   mkdir build && cd build
   cmake ..
   ```

3. Build the plugin:

   ```bash
   cmake --build . --config Release
   ```

4. Locate the generated `.vst3` file:

   - On Windows: `build/ParamEQ_artefacts/Release/VST3/ParamEQ.vst3`
   - On macOS: `build/ParamEQ_artefacts/Release/ParamEQ.vst3`
   - On Linux: varies depending on configuration

5. Copy the `.vst3` file to your DAW's VST3 plugin folder.

---

## License

This project is licensed under the GNU Affero General Public License v3.0 (AGPLv3).  
See the LICENSE file for full terms.

---

## Acknowledgements

- Built with [JUCE](https://juce.com)  
- Dependency management via [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)  
- Based on [JuceAudioPluginTemplate](https://github.com/TheAudioProgrammer/JuceAudioPluginTemplate)  
- Developed as part of an academic final project

---

# ParamEQ (em português)

Plugin equalizador paramétrico leve e personalizável, desenvolvido em C++ com JUCE.  
Projetado para processamento de áudio em tempo real em DAWs compatíveis com o padrão VST3.

---

## Funcionalidades

- 8 bandas de equalização independentes  
- Tipos de filtro: Peak, Shelf (alta e baixa), Passa-altas e Passa-baixas  
- Curva de equalização e espectro do áudio exibidos em tempo real  
- Interface gráfica responsiva e otimizada  
- Suporte a áudio estéreo  
- Compatível com automação de parâmetros via DAW  
- Salva e restaura os parâmetros com a sessão do projeto  

---

## Branches

### `main`

- 🔄 Branch principal de desenvolvimento  
- 🛠️ Onde ocorrem melhorias e novas funcionalidades  
- 💬 Aberta a contribuições externas via pull request (sujeitas à aprovação)

### `tcc-final`

- 📌 Contém o estado exato do projeto no momento da entrega do projeto acadêmico final  
- 🧊 Permanentemente congelada e protegida contra alterações  
- 📎 Mantida como versão de referência acadêmica

---

## Como compilar o plugin

O projeto utiliza JUCE (v8.0.8) e CMake para automatizar a construção do plugin. A dependência da JUCE é baixada automaticamente.

### 🔧 Requisitos

- CMake 3.15 ou superior  
- Compilador compatível com C++17 (GCC, Clang ou MSVC)  
- Git  
- DAW compatível com VST3 para testes (ex: Ableton Live, Reaper, AudioPluginHost)  

### 📦 Etapas de compilação (Windows/Linux/macOS)

1. Clone o repositório:

   ```bash
   git clone --recursive https://github.com/seu-usuario/ParamEQ.git
   cd ParamEQ
   ```

2. Crie a pasta de build e execute o CMake:

   ```bash
   mkdir build && cd build
   cmake ..
   ```

3. Compile o plugin:

   ```bash
   cmake --build . --config Release
   ```

4. Localize o arquivo `.vst3` gerado:

   - No Windows: `build/ParamEQ_artefacts/Release/VST3/ParamEQ.vst3`
   - No macOS: `build/ParamEQ_artefacts/Release/ParamEQ.vst3`
   - No Linux: depende da configuração do sistema

5. Copie o `.vst3` para a pasta de plugins da sua DAW.

---

## Licença

Este projeto está licenciado sob a GNU AGPLv3.  
Consulte o arquivo LICENSE para mais informações.

---

## Agradecimentos

- Construído com [JUCE](https://juce.com)  
- Gerenciamento de dependências via [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)  
- Baseado em [JuceAudioPluginTemplate](https://github.com/TheAudioProgrammer/JuceAudioPluginTemplate)  
- Desenvolvido como parte de um Trabalho de Conclusão de Curso (TCC)
