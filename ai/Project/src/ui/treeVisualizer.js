// Interactive Canvas-based Minimax / Expectiminimax Game Tree Visualizer
export class GameTreeVisualizer {
  constructor(canvasContainerEl) {
    this.container = canvasContainerEl;
    this.canvas = document.createElement('canvas');
    this.ctx = this.canvas.getContext('2d');
    this.container.appendChild(this.canvas);

    this.treeData = null;
    this.selectedNode = null;
    this.nodes = []; // Flat list with computed layout (x, y, w, h)
    this.edges = [];

    this.resize();
    window.addEventListener('resize', () => this.resize());
    this.setupEvents();
  }

  resize() {
    this.width = this.container.clientWidth || 600;
    this.height = this.container.clientHeight || 420;
    this.canvas.width = this.width * window.devicePixelRatio;
    this.canvas.height = this.height * window.devicePixelRatio;
    this.canvas.style.width = `${this.width}px`;
    this.canvas.style.height = `${this.height}px`;
    this.ctx.scale(window.devicePixelRatio, window.devicePixelRatio);
    this.render();
  }

  setTree(treeData, explanation = '') {
    this.treeData = treeData;
    this.explanation = explanation;
    this.selectedNode = null;
    this.computeLayout();
    this.render();
  }

  // Compute hierarchical layout: Level 0 (Root), Level 1 (Actions: Buy/Pass), Level 2 (Chance rolls)
  computeLayout() {
    this.nodes = [];
    this.edges = [];
    if (!this.treeData) return;

    const levelY = [55, 170, 310];
    const rootX = this.width / 2;

    // Root Node (MAX)
    const root = {
      id: 'root',
      text: this.treeData.name || 'Decision Point',
      subtext: 'MAX Node',
      type: 'MAX',
      x: rootX,
      y: levelY[0],
      w: 180,
      h: 52,
      chosen: true,
      data: this.treeData
    };
    this.nodes.push(root);

    // Level 1: Action Branches (BUY vs PASS)
    const actions = this.treeData.children || [];
    if (actions.length === 0) return;

    const actionSpacing = Math.min(260, this.width / (actions.length + 1));
    const startActionX = rootX - ((actions.length - 1) * actionSpacing) / 2;

    actions.forEach((act, idx) => {
      const actX = startActionX + idx * actionSpacing;
      const actNode = {
        id: `act_${idx}`,
        text: act.action,
        subtext: `EV: ${act.score || 0}`,
        type: 'CHANCE',
        x: actX,
        y: levelY[1],
        w: 170,
        h: 56,
        chosen: act.chosen || false,
        score: act.score,
        data: act
      };
      this.nodes.push(actNode);
      this.edges.push({ from: root, to: actNode, chosen: act.chosen });

      // Level 2: Chance dice rolls under this action
      const rolls = act.children || [];
      if (rolls.length > 0) {
        const rollSpacing = Math.min(105, (this.width / actions.length) / (rolls.length + 0.5));
        const startRollX = actX - ((rolls.length - 1) * rollSpacing) / 2;

        rolls.forEach((r, rIdx) => {
          const rollX = startRollX + rIdx * rollSpacing;
          const leafNode = {
            id: `roll_${idx}_${rIdx}`,
            text: r.action,
            subtext: `Score: ${r.score}`,
            type: 'LEAF',
            x: rollX,
            y: levelY[2],
            w: 96,
            h: 48,
            chosen: act.chosen,
            data: r
          };
          this.nodes.push(leafNode);
          this.edges.push({ from: actNode, to: leafNode, chosen: act.chosen });
        });
      }
    });
  }

  render() {
    this.ctx.clearRect(0, 0, this.width, this.height);

    if (!this.treeData) {
      // Empty state
      this.ctx.fillStyle = '#64748b';
      this.ctx.font = '500 15px "Inter", sans-serif';
      this.ctx.textAlign = 'center';
      this.ctx.fillText('Game tree updates dynamically during AI decision turns...', this.width / 2, this.height / 2);
      return;
    }

    // 1. Draw Edges
    this.edges.forEach(edge => {
      this.ctx.beginPath();
      this.ctx.moveTo(edge.from.x, edge.from.y + edge.from.h / 2);

      // Curved bezier path
      const cp1y = (edge.from.y + edge.to.y) / 2;
      this.ctx.bezierCurveTo(edge.from.x, cp1y, edge.to.x, cp1y, edge.to.x, edge.to.y - edge.to.h / 2);

      if (edge.chosen) {
        this.ctx.strokeStyle = '#10b981'; // Glowing Emerald for chosen branch
        this.ctx.lineWidth = 3.5;
        this.ctx.shadowColor = 'rgba(16, 185, 129, 0.4)';
        this.ctx.shadowBlur = 8;
      } else {
        this.ctx.strokeStyle = 'rgba(100, 116, 139, 0.35)';
        this.ctx.lineWidth = 1.5;
        this.ctx.shadowBlur = 0;
      }
      this.ctx.stroke();
    });
    this.ctx.shadowBlur = 0;

    // 2. Draw Nodes
    this.nodes.forEach(node => {
      const isSelected = this.selectedNode && this.selectedNode.id === node.id;
      const isChosen = node.chosen;

      const left = node.x - node.w / 2;
      const top = node.y - node.h / 2;

      // Card Background
      this.ctx.beginPath();
      this.ctx.roundRect(left, top, node.w, node.h, 8);

      if (isChosen) {
        this.ctx.fillStyle = node.type === 'MAX' ? '#064e3b' : '#065f46';
        this.ctx.strokeStyle = isSelected ? '#34d399' : '#10b981';
        this.ctx.lineWidth = isSelected ? 3 : 2;
      } else {
        this.ctx.fillStyle = '#1e293b';
        this.ctx.strokeStyle = isSelected ? '#94a3b8' : '#334155';
        this.ctx.lineWidth = isSelected ? 2 : 1;
      }

      this.ctx.fill();
      this.ctx.stroke();

      // Node text
      this.ctx.fillStyle = isChosen ? '#ecfdf5' : '#f8fafc';
      this.ctx.font = node.type === 'LEAF' ? 'bold 10px "Inter", sans-serif' : 'bold 12px "Inter", sans-serif';
      this.ctx.textAlign = 'center';
      this.ctx.textBaseline = 'middle';

      // Text truncation if needed
      const maxTextW = node.w - 14;
      let displayTxt = node.text;
      if (this.ctx.measureText(displayTxt).width > maxTextW) {
        while (this.ctx.measureText(displayTxt + '...').width > maxTextW && displayTxt.length > 4) {
          displayTxt = displayTxt.slice(0, -1);
        }
        displayTxt += '...';
      }

      this.ctx.fillText(displayTxt, node.x, node.y - 7);

      // Subtext / Score badge
      this.ctx.fillStyle = isChosen ? '#6ee7b7' : '#94a3b8';
      this.ctx.font = '10px "Inter", monospace';
      this.ctx.fillText(node.subtext, node.x, node.y + 11);
    });

    // 3. Explanation banner at top-left
    if (this.explanation) {
      this.ctx.fillStyle = 'rgba(15, 23, 42, 0.85)';
      this.ctx.beginPath();
      this.ctx.roundRect(10, 8, this.width - 20, 26, 6);
      this.ctx.fill();

      this.ctx.fillStyle = '#38bdf8';
      this.ctx.font = '600 11px "Inter", sans-serif';
      this.ctx.textAlign = 'left';
      this.ctx.textBaseline = 'middle';
      this.ctx.fillText(`⚡ ${this.explanation}`, 20, 21);
    }
  }

  setupEvents() {
    this.canvas.addEventListener('click', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      const clickX = e.clientX - rect.left;
      const clickY = e.clientY - rect.top;

      const hit = this.nodes.find(n =>
        clickX >= n.x - n.w / 2 &&
        clickX <= n.x + n.w / 2 &&
        clickY >= n.y - n.h / 2 &&
        clickY <= n.y + n.h / 2
      );

      if (hit) {
        this.selectedNode = hit;
        this.render();
        if (this.onNodeSelected) {
          this.onNodeSelected(hit);
        }
      }
    });
  }
}
