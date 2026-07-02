import subprocess
import re
import os
import sys

def run_benchmark():
    # 1. Compilar o programa
    print("Compilando ping_pong.c...")
    compile_cmd = ["mpicc", "ping_pong.c", "-o", "ping_pong"]
    try:
        subprocess.run(compile_cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Erro ao compilar: {e}")
        return None
    except FileNotFoundError:
        print("Erro: 'mpicc' não encontrado. Certifique-se de que o MPI está instalado corretamente.")
        return None

    # 2. Executar o benchmark
    print("Executando o benchmark MPI com 2 processos...")
    run_cmd = ["mpirun", "--oversubscribe", "-np", "2", "./ping_pong"]
    # Nota: Usamos --oversubscribe caso a máquina tenha apenas 1 núcleo físico disponível para teste.
    try:
        result = subprocess.run(run_cmd, capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        # Se falhar sem --oversubscribe, tenta sem a flag
        try:
            run_cmd = ["mpirun", "-np", "2", "./ping_pong"]
            result = subprocess.run(run_cmd, capture_output=True, text=True, check=True)
            return result.stdout
        except subprocess.CalledProcessError as e2:
            print(f"Erro ao rodar o benchmark: {e2}")
            print(f"Saída de erro: {e2.stderr}")
            return None
    except FileNotFoundError:
        print("Erro: 'mpirun' não encontrado.")
        return None

def parse_data(output):
    sizes = []
    iterations = []
    times = []
    latencies = []
    bandwidths = []

    print("\n--- Resultados do Benchmark ---")
    print(output)

    # Expressão regular para capturar linhas de dados
    # Ex: 8            10000        0.002345        0.117           136.321
    pattern = re.compile(r"^\s*(\d+)\s+(\d+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)")
    for line in output.split('\n'):
        match = pattern.match(line)
        if match:
            sizes.append(int(match.group(1)))
            iterations.append(int(match.group(2)))
            times.append(float(match.group(3)))
            latencies.append(float(match.group(4)))
            bandwidths.append(float(match.group(5)))
            
    return sizes, latencies, bandwidths

def plot_data(sizes, latencies, bandwidths):
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("\n[AVISO] Biblioteca 'matplotlib' não encontrada.")
        print("Para gerar o gráfico, instale-a rodando:")
        print("    sudo apt install python3-matplotlib")
        print("ou")
        print("    pip install matplotlib")
        return False

    # Criando o gráfico
    fig, ax1 = plt.subplots(figsize=(10, 6))

    # Plot da Latência (Eixo Y1 - Esquerdo, escala logarítmica para ver os extremos)
    color = 'tab:red'
    ax1.set_xlabel('Tamanho da Mensagem (Bytes)', fontweight='bold')
    ax1.set_ylabel('Latência Unidirecional (µs)', color=color, fontweight='bold')
    # Usando escala log no eixo X (tamanhos de mensagem) e log no eixo Y (latência)
    ax1.set_xscale('log', base=2)
    ax1.set_yscale('log')
    line1 = ax1.plot(sizes, latencies, marker='o', color=color, linewidth=2, label='Latência (µs)')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.grid(True, which="both", ls="--", alpha=0.5)

    # Segundo eixo Y para a Largura de Banda
    ax2 = ax1.twinx()
    color = 'tab:blue'
    ax2.set_ylabel('Largura de Banda (MB/s)', color=color, fontweight='bold')
    # Largura de banda em escala linear ou log
    line2 = ax2.plot(sizes, bandwidths, marker='s', color=color, linewidth=2, label='Largura de Banda (MB/s)')
    ax2.tick_params(axis='y', labelcolor=color)

    # Adicionar legenda unificada
    lines = line1 + line2
    labels = [l.get_label() for l in lines]
    ax1.legend(lines, labels, loc='upper left')

    plt.title('Benchmark MPI Ping-Pong: Latência vs. Largura de Banda', fontsize=14, fontweight='bold', pad=15)
    fig.tight_layout()
    
    # Salvar imagem
    output_filename = 'ping_pong_performance.png'
    plt.savefig(output_filename, dpi=150)
    print(f"\n[SUCESSO] Gráfico gerado com sucesso e salvo em: '{output_filename}'")
    return True

def main():
    output = run_benchmark()
    if output:
        sizes, latencies, bandwidths = parse_data(output)
        if sizes:
            plot_data(sizes, latencies, bandwidths)
            
            # Análise teórica baseada no comportamento padrão
            print("\n--- Análise dos Resultados ---")
            print("1. Região dominada por Latência:")
            print("   - Para mensagens pequenas (ex: 8 B até ~1 KB ou 4 KB), a latência permanece quase constante e baixa.")
            print("   - Nessa região, o tempo de transmissão é dominado pelo overhead de inicialização (startup time / latency).")
            print("   - A largura de banda obtida é extremamente baixa porque o overhead da rede/software domina o tempo total.")
            print("\n2. Região dominada por Largura de Banda:")
            print("   - Para mensagens maiores (ex: acima de 64 KB até 4 MB), a latência aumenta de forma linear com o tamanho.")
            print("   - A largura de banda cresce e se estabiliza (atinge o platô próximo do limite físico de loopback ou rede).")
            print("   - Nessa região, o tempo de transmissão é dominado pelo tempo necessário para empacotar e enviar os dados físicos.")

if __name__ == "__main__":
    main()
