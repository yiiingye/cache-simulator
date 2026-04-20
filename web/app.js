let chart = null;

document.getElementById("themeToggle").addEventListener("change", e => {
    document.body.className = e.target.checked ? "dark" : "light";
    if (lastResult) {
        updateChart(lastResult);
        updateCounts(lastResult);
    }
});

function getThemeColors() {
    const isDark = document.body.classList.contains("dark");

    return {
        hit: isDark ? "#22d37f" : "#00c16a",
        miss: isDark ? "#ff6b6d" : "#ff4d4f",
        replacement: isDark ? "#ffbf47" : "#ffb020",
        cycles: isDark ? "#5c8dff" : "#2f6bff",
        time: isDark ? "#a27bff" : "#8b5cff",
        grid: isDark ? "rgba(165, 174, 187, 0.12)" : "rgba(99, 115, 139, 0.14)",
        tick: isDark ? "#d7dde6" : "#66748a",
        title: isDark ? "#fafafa" : "#162032"
    };
}

let lastResult = null;

function updateInterfaceState(hasResults) {
    document.getElementById("chartContainer").style.display = hasResults ? "block" : "none";
    document.getElementById("counts").style.display = hasResults ? "grid" : "none";
    document.getElementById("emptyState").style.display = hasResults ? "none" : "block";
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
            lastResult = data;
            updateInterfaceState(true);
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
    const hitPercent = totalAccesses ? Number((data.hits / totalAccesses * 100).toFixed(2)) : 0;
    const missPercent = totalAccesses ? Number((data.misses / totalAccesses * 100).toFixed(2)) : 0;

    chart = new Chart(ctx, {
        type: "bar",
        data: {
            labels: ["Hit Rate", "Miss Rate"],
            datasets: [{
                data: [hitPercent, missPercent],
                backgroundColor: [colors.hit, colors.miss],
                borderRadius: 8,
                borderSkipped: false
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            layout: { padding: { top: 8, bottom: 6 } },
            plugins: {
                legend: { display: false },
                title: {
                    display: true,
                    text: "Outcome Distribution",
                    color: colors.title,
                    font: { size: 16, weight: "600", family: "Aptos, Segoe UI, Arial, sans-serif" },
                    padding: { bottom: 20 }
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
                        color: colors.grid,
                        drawBorder: false
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
                        color: colors.tick,
                        font: { family: "Aptos, Segoe UI, Arial, sans-serif", size: 12, weight: "600" }
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
    const hitRate = totalAccesses
        ? (data.hits / totalAccesses * 100).toFixed(2)
        : "0.00";
    const totalCycles = Number(data.totalCycles).toFixed(2);
    const amat = Number(data.amat).toFixed(3);
    const estimatedTimeNs = Number(data.estimatedTimeNs).toFixed(2);
    const estimatedTimeUs = (Number(data.estimatedTimeNs) / 1000).toFixed(2);

    countsDiv.innerHTML = `
        <div class="count-item">
            <span>Hit Rate</span>
            <div class="count-value" style="color:${colors.hit}">${hitRate}%</div>
            <div class="count-meta">${data.hits} hits across ${totalAccesses} accesses</div>
        </div>

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
            <div class="count-meta">${replacementRate}% of total accesses</div>
        </div>

        <div class="count-item">
            <span>Total Cycles</span>
            <div class="count-value" style="color:${colors.cycles}">${totalCycles}</div>
            <div class="count-meta">AMAT ${amat} cycles per access</div>
        </div>

        <div class="count-item">
            <span>Estimated Time</span>
            <div class="count-value" style="color:${colors.time}">${estimatedTimeNs} ns</div>
            <div class="count-meta">${estimatedTimeUs} μs at ${Number(data.cpuFrequencyGHz).toFixed(2)} GHz</div>
        </div>
    `;
}

const cacheTypeSelect = document.getElementById("cacheType");
const policyField = document.getElementById("policyField");

function syncPolicyVisibility() {
    if (cacheTypeSelect.value === "DM") {
        policyField.style.display = "none";
    } else {
        policyField.style.display = "flex";
    }
}

cacheTypeSelect.addEventListener("change", syncPolicyVisibility);
syncPolicyVisibility();
updateInterfaceState(false);
