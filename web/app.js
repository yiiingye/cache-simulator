let chart = null;

document.getElementById("themeToggle").addEventListener("change", e => {
    document.body.className = e.target.checked ? "dark" : "light";
});

function getThemeColors() {
    const isDark = document.body.classList.contains("dark");

    return {
        hit: isDark ? "#a6e22e" : "#7fb414",
        miss: isDark ? "#c61515ff" : "#c61515ff",
        replacement: "#fd971f",
        cycles: "#66d9ef",
        time: "#ae81ff",
        grid: isDark ? "rgba(248, 248, 242, 0.12)" : "rgba(47, 49, 41, 0.14)",
        tick: isDark ? "#cfcfbe" : "#4f5244",
        title: isDark ? "#f8f8f2" : "#33362d"
    };
}


function runSimulation() {
    const cacheType = document.getElementById("cacheType").value;
    const policy = document.getElementById("policy").value;
    const pattern = document.getElementById("pattern").value;
    const numAccesses = document.getElementById("numAccesses").value;
    const hitLatency = document.getElementById("hitLatency").value;
    const missPenalty = document.getElementById("missPenalty").value;
    const cpuFrequencyGHz = document.getElementById("cpuFrequencyGHz").value;

    const params = new URLSearchParams({
        cacheType,
        policy,
        pattern,
        numAccesses,
        hitLatency,
        missPenalty,
        cpuFrequencyGHz
    });
    const url = `http://localhost:8080/simulate?${params.toString()}`;

    fetch(url)
        .then(res => {
            if (!res.ok) {
                throw new Error("Simulation request failed");
            }
            return res.json();
        })
        .then(data => {
            document.getElementById("chartContainer").style.display = "flex";
            updateChart(data);
            updateCounts(data);
        })
        .catch(error => {
            console.error(error);
            alert("Unable to run the simulation. Please check that the backend is running.");
        });
}

function updateChart(data) {
    const ctx = document.getElementById("chart").getContext("2d");
    const colors = getThemeColors();

    if (chart) chart.destroy();

    const totalAccesses = data.hits + data.misses;
    const hitPercent = totalAccesses ? Number((data.hits / totalAccesses * 100).toFixed(3)) : 0;
    const missPercent = totalAccesses ? Number((data.misses / totalAccesses * 100).toFixed(3)) : 0;

    chart = new Chart(ctx, {
        type: "bar",
        data: {
            labels: ["Hit Rate", "Miss Rate"],
            datasets: [{
                data: [hitPercent, missPercent],
                backgroundColor: [colors.hit, colors.miss],
                borderRadius: 6
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            layout: { padding: { top: 10, bottom: 10 } },
            plugins: {
                legend: { display: false },
                title: {
                    display: true,
                    text: "Access Outcome Distribution",
                    color: colors.title,
                    font: { size: 16, weight: "600" }
                },
                tooltip: {
                    callbacks: {
                        label: function(context) {
                            return `${context.parsed.y}%`;
                        }
                    }
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    max: 100,
                    grid: {
                        color: colors.grid
                    },
                    ticks: {
                        color: colors.tick,
                        callback: value => value + "%"
                    }
                },
                x: {
                    grid: {
                        display: false
                    },
                    ticks: {
                        color: colors.tick
                    }
                }
            }
        }
    });
}

function updateCounts(data) {
    const countsDiv = document.getElementById("counts");
    const colors = getThemeColors();
    const totalAccesses = data.hits + data.misses;
    const replacementRate = totalAccesses
        ? (data.replacements / totalAccesses * 100).toFixed(2)
        : "0.00";
    const totalCycles = Number(data.totalCycles).toFixed(2);
    const amat = Number(data.amat).toFixed(3);
    const estimatedTimeNs = Number(data.estimatedTimeNs).toFixed(2);
    const estimatedTimeUs = (Number(data.estimatedTimeNs) / 1000).toFixed(2);

    countsDiv.innerHTML = `
        <div class="count-item">
            <span>Hits</span>
            <div class="count-value" style="color:${colors.hit}">${data.hits}</div>
        </div>

        <div class="count-item">
            <span>Misses</span>
            <div class="count-value" style="color:${colors.miss}">${data.misses}</div>
        </div>

        <div class="count-item">
            <span>Replacements</span>
            <div class="count-value" style="color:${colors.replacement}">${data.replacements}</div>
            <div class="count-meta">${replacementRate}% of accesses</div>
        </div>

        <div class="count-item">
            <span>Total Cycles</span>
            <div class="count-value" style="color:${colors.cycles}">${totalCycles}</div>
            <div class="count-meta">AMAT ${amat} cycles/access</div>
        </div>

        <div class="count-item">
            <span>Estimated Time</span>
            <div class="count-value" style="color:${colors.time}">${estimatedTimeNs} ns</div>
            <div class="count-meta">${estimatedTimeUs} μs at ${Number(data.cpuFrequencyGHz).toFixed(2)} GHz</div>
        </div>
    `;
}

const cacheTypeSelect = document.getElementById("cacheType");
const policyLabel = document.querySelector("label[for='policy']");
const policySelect = document.getElementById("policy");

cacheTypeSelect.addEventListener("change", () => {
    if (cacheTypeSelect.value === "DM") {
        policyLabel.style.display = "none";
        policySelect.style.display = "none";
    } else {
        policyLabel.style.display = "block";
        policySelect.style.display = "block";
    }
});
