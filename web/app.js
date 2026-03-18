let chart = null;

document.getElementById("themeToggle").addEventListener("change", e => {
    document.body.className = e.target.checked ? "dark" : "light";
});

function runSimulation() {
    const cacheType = document.getElementById("cacheType").value;
    const policy = document.getElementById("policy").value;
    const pattern = document.getElementById("pattern").value;
    const numAccesses = document.getElementById("numAccesses").value;

    const url = `http://localhost:8080/simulate?cacheType=${cacheType}&policy=${policy}&pattern=${pattern}&numAccesses=${numAccesses}`;

    fetch(url)
        .then(res => res.json())
        .then(data => {
            document.getElementById("chartContainer").style.display = "block";
            updateChart(data);
        });
}

function updateChart(data) {
    const ctx = document.getElementById("chart").getContext("2d");

    if (chart) chart.destroy();

    chart = new Chart(ctx, {
        type: "bar",
        data: {
            labels: ["Hits", "Misses", "Hit Rate (%)", "Replacements"],
            datasets: [{
                data: [data.hits, data.misses, data.hitRate, data.replacements],
                backgroundColor: ["#4caf50", "#f44336", "#2196f3", "#ff9800"],
                borderRadius: 6
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false },
                title: {
                    display: true,
                    text: "Simulation Results",
                    font: { size: 18 }
                }
            }
        }
    });
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
